/* tosser.cc - tosser for tanstaafl */
/* Copyright 1998-1999 Per Lundberg */

#include "tanstaafl.h"
#include "crc32.h"
#include "compress.h"

#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

TOSSER::TOSSER (void)
{
}

TOSSER::~TOSSER (void)
{
}

/* Toss the specified filename. Should be rewritten to read the whole file in
   a single pass. Works, but could be optimized more. I could need help with
   this. 

   Returns the number of messages tossed, or -1 if error */

sword TOSSER::toss (char *filename, Window *win)
{
  FILE *file;
  pkthdr_type pkthdr;
  packed_msg_type msg;
  msghdr_type msghdr;
  char tmpstr[256], *tmpstr2;
  word c = 0, d = 0;
  char *buffer = new char[1024 * 256];
  char area[256];
  word msgs = 0;
  bool netmail = FALSE;
  byte aka;

  msgbase.init (config.cfg.msgbase_path);

  file = fopen (filename, "rb");
  if (file == NULL)
    {
      ui.infobox ("Error", string ("Could not open ") + string (filename));
      delete buffer;
      buffer = NULL;
      return -1;
    }

  fread (&pkthdr, sizeof (pkthdr_type), 1, file);
  
  if (pkthdr.packetVersion != 2) /* we only support Type 2+ */
    {
      delete buffer;
      buffer = NULL;
      return -1;
    }

  while (config.cfg.aka[c].zone) 
    {
      if (pkthdr.destZone == config.cfg.aka[c].zone &&
	  pkthdr.destNet == config.cfg.aka[c].net &&
	  pkthdr.destNode == config.cfg.aka[c].node &&
	  pkthdr.destPoint == config.cfg.aka[c].point)
	break;
      c++;
    }
  
  if (config.cfg.aka[c].zone)
    {
      aka = c;
      if (pkthdr.origPoint)
	sprintf (tmpstr, "%s %u:%u/%u.%u -> %u:%u/%u.%u", filename, 
		 pkthdr.origZone, pkthdr.origNet, pkthdr.origNode,
		 pkthdr.origPoint, pkthdr.destZone, pkthdr.destNet,
		 pkthdr.destNode, pkthdr.destPoint);
      else
	sprintf (tmpstr, "%s %u:%u/%u -> %u:%u/%u.%u", filename, 
		 pkthdr.origZone, pkthdr.origNet, pkthdr.origNode,
		 pkthdr.destZone, pkthdr.destNet,
		 pkthdr.destNode, pkthdr.destPoint);
   
      win->put (2, 1, tmpstr, config.cfg.color[MISC_NORMAL]);

      for (c = strlen (tmpstr) + 2; c < 69; c++)
	win->put (c, 1, ' ', config.cfg.color[MISC_NORMAL]);

      while (!feof (file))
	{
	  memset (&msghdr, 0, sizeof (msghdr_type));
	  fread (&msg, sizeof (packed_msg_type), 1, file);
	  if (!feof (file) && msg.id)
	    {
	      fgetsz (file, (char *) msghdr.recipient, 36);
	      fgetsz (file, (char *) msghdr.sender, 36);
	      fgetsz (file, (char *) msghdr.subject, 72);
	      fgetsz (file, buffer, 256 * 1024 - 1); /* message text */

	      if (!strncmp (buffer, "AREA:", 5)) /* Echomail */
		{
		  for (c = 0; buffer[c + 5] != 0x0D; c++);
		  strncpy (area, buffer + 5, c);
		  area[c] = 0;
		  c = 6 + strlen (area);
		  d = strlen (buffer);
		  memcpy (buffer, buffer + c, d - c);
		  buffer[d - c] = 0;
		  
		  /* Determine the folder number */

		  msghdr.folder = 65535;
		  for (c = 0; c < config.area_header.areas; c++)
		    {
		      if (!strcasecmp (area, config.area[c]->name))
			{
			  msghdr.folder = c;
			  break;
			}
		    }

		  if (msghdr.folder == 65535) /* Area does not exist */
		    {
#ifdef DEBUG
		      printf ("%s %s\n", area, config.area[c]->name);
		      keyboard.get ();
#endif
		      area_type new_area;
		      
		      memset (&new_area, 0, sizeof (area_type));
		      
		      new_area.aka = aka;
		      for (c = 0; c < 10; c++)
			if (config.cfg.uplink[c].zone == pkthdr.origZone)
			  new_area.uplink = c;
		      strcpy (new_area.name, area);
		      strcpy (new_area.comment, area);

		      msghdr.folder = config.add_area (new_area);
		    }
		}
	      else /* Netmail */
		{
		  area[0] = 0;
		  netmail = TRUE;
		  msghdr.status_flags |= FLAG_PRIVATE; /* Private */
		  msghdr.folder = 1;
		  msghdr.recipient_addr.point = 
		    atoi (get_string (buffer, "\001TOPT"));
		  msghdr.sender_addr.point =
		    atoi (get_string (buffer, "\001FMPT"));

		  // FIXME: should also check for INTL kludge

		  msghdr.recipient_addr.zone = pkthdr.destZone;
		  msghdr.recipient_addr.net = msg.dest_net;
		  msghdr.recipient_addr.node = msg.dest_node;
		}

	      /* Save the CRC for the MSGID value */

	      tmpstr2 = get_string (buffer, "\001MSGID: ");
	      if (!tmpstr2[0]) 
		msghdr.msgid_CRC = 0;
	      else
		msghdr.msgid_CRC = crc32buf (tmpstr2, strlen (tmpstr2));
#ifdef DEBUG
	      printf ("%s 0x%08X ", msghdr.recipient, msghdr.msgid_CRC);
#endif

	      /* ...and the REPLY value */
	      
	      tmpstr2 = get_string (buffer, "\001REPLY: ");
	      if (!tmpstr2[0])
		msghdr.reply_CRC = 0;
	      else
		msghdr.reply_CRC = crc32buf (tmpstr2, strlen (tmpstr2));
#ifdef DEBUG
	      printf ("0x%08X\n", msghdr.reply_CRC);
	      keyboard.get ();
#endif
	
	      /* We do not link right now; that is being done later on */

	      msghdr.reply_mother = msghdr.reply_child = msghdr.reply_prev = msghdr.reply_next = MAX;

	      /* Parse the time of the message */

	      struct tm tblock, *tblock2;
	      time_t t;

	      time (&t);
	      tblock2 = localtime (&t);
	      tblock.tm_mday = atoi (msg.date);
	      strncpy (tmpstr, msg.date + 3, 3);
	      tmpstr[3] = 0;
	      for (c = 0; c < 12; c++)
		if (!strcmp (month[c], tmpstr))
		  tblock.tm_mon = c;
	      tblock.tm_year = atoi (msg.date + 7);
	      if (tblock.tm_year < 80) /* 20xx */
		tblock.tm_year += 100;
	      tblock.tm_hour = atoi (msg.date + 11);
	      tblock.tm_min = atoi (msg.date + 14);
	      tblock.tm_sec = atoi (msg.date + 17);
	      tblock.tm_gmtoff = 0;
	      tblock.tm_isdst = 0;  /* is this daylight saving time? */
	      
	      msghdr.time_written = mktime (&tblock);
	      msghdr.time_written += tblock2->tm_gmtoff; /* time_written is to
							    be stored as local
							    time */

	      /* Determine the sender address by the Origin line */

	      address_type addr;

	      strcpy (tmpstr, get_string (buffer, "\r * Origin:"));
	      c = strlen (tmpstr);
	      while (tmpstr[c] != '(' && c) c--;
	      addr = strToBinAddr (tmpstr + c + 1);

	      if (addr.zone) 
		{
		  msghdr.sender_addr.zone = addr.zone;
		  msghdr.sender_addr.net = addr.net;
		  msghdr.sender_addr.node = addr.node;
		  msghdr.sender_addr.point = addr.point;
		}
	      else /* No origin line, so fall back to MSGID */
		{
		  addr = strToBinAddr (get_string (buffer, "\001MSGID: "));
		  if (addr.zone) /* Perhaps no MSGID either? */
		    msghdr.sender_addr = addr;
		  else
		  {
		    msghdr.sender_addr.zone = pkthdr.origZone;
		    msghdr.sender_addr.net = msg.orig_net;
		    msghdr.sender_addr.node = msg.orig_node;
		    msghdr.sender_addr.point = 0;
		  }
		}

	      /* Convert charset if required */

	      tmpstr2 = get_string (buffer, "\001CHRS: ");
	      if (!strncmp (tmpstr2, "LATIN-1 2", 9))
		{
		  convert_charset ((char *) buffer, 1, 0);
		  convert_charset ((char *) msghdr.sender, 1, 0);
		  convert_charset ((char *) msghdr.recipient, 1, 0);
		  convert_charset ((char *) msghdr.subject, 1, 0);
		}

	      msghdr.txt_size = strlen (buffer);

	      msgbase.write_message (msghdr, buffer);
	      msgs++;

	      if (!netmail)
		sprintf (tmpstr, "Message from %s in %s", msghdr.sender,
			 area);
	      else
		sprintf (tmpstr, "Netmail from %s (%u:%u/%u.%u)", 
			 msghdr.sender, msghdr.sender_addr.zone,
			 msghdr.sender_addr.net, msghdr.sender_addr.node,
			 msghdr.sender_addr.point);
	      win->put (4, 2, tmpstr, config.cfg.color[MISC_NORMAL]);
	      for (c = strlen (tmpstr) + 4; c < 69; c++)
		win->put (c, 2, ' ', config.cfg.color[MISC_NORMAL]);
	      screen.refresh ();
	    }
	}
      fclose (file);
      unlink (filename);
    }
  else
    {
      fclose (file);
    }
  delete buffer;
  buffer = NULL;
  return msgs;
}

/* Main tosser part */

void TOSSER::play (byte flags)
{
  DIR *dir;
  struct dirent *ent;
  Window *win;
  char tmpstr[256];
  dword c;
  dword msgs = 0;
  time_t t1, t2;

  win = new Window (screen.x_max / 2 - 35, screen.y_max / 2 - 4, 70, 8, config.cfg.color[MISC_WINDOW], config.cfg.color[MISC_TITLE],
		    "Tossing...");
  screen.refresh ();
  chdir (config.cfg.inbound_path);

  /* Unpack ARCMail bundles */

  dir = opendir (config.cfg.inbound_path);

  do
    {
      ent = readdir (dir);
      if (ent != NULL)
	{
	  if ((strstr(ent->d_name, ".mo") ||
	       strstr(ent->d_name, ".tu") ||
	       strstr(ent->d_name, ".we") ||
	       strstr(ent->d_name, ".th") ||
	       strstr(ent->d_name, ".fr") ||
	       strstr(ent->d_name, ".sa") ||
	       strstr(ent->d_name, ".su") ||
	       strstr(ent->d_name, ".MO") ||
	       strstr(ent->d_name, ".TU") ||
	       strstr(ent->d_name, ".WE") ||
	       strstr(ent->d_name, ".TH") ||
	       strstr(ent->d_name, ".FR") ||
	       strstr(ent->d_name, ".SA") ||
	       strstr(ent->d_name, ".SU")) &&
	      strlen(ent->d_name) == 12 &&
	      ent->d_name[11] >= '0' && ent->d_name[11] <= '9')
	    {
	      sprintf (tmpstr, "Unpacking %s...       ", ent->d_name);
	      win->put (2, 1, tmpstr, config.cfg.color[MISC_NORMAL]);

	      if (!uncompress ((byte *) ent->d_name))
		{
		  win->put (15 + strlen (ent->d_name), 1, "done", config.cfg.color[MISC_NORMAL]);
		  sprintf (tmpstr, "%s/%s", config.cfg.inbound_path,
			   ent->d_name);
		  unlink (tmpstr);
		}
	      else
		win->put (15 + strlen (ent->d_name), 1, "error!", config.cfg.color[MISC_NORMAL]);
	      screen.refresh ();

	    }
	}
    } while (ent != NULL);
  closedir (dir);

  /* Toss the PKT files */

  time (&t1);

  dir = opendir (config.cfg.inbound_path);
  do
    {
      ent = readdir (dir);
      if (ent != NULL)
	{
	  if (strstr (ent->d_name, ".pkt") ||
	      strstr (ent->d_name, ".PKT"))
	    msgs += toss (ent->d_name, win);
	}
    } while (ent != NULL);
  closedir (dir);

  time (&t2);
  
  if (t2 == t1)
    t2++;
  if (msgs)
    {
      sprintf (tmpstr, "Tossed %lu messages in %lu seconds. (%lu msgs/s)",
	       msgs, t2 - t1, msgs / (t2 - t1));
      win->put (2, 4, tmpstr, config.cfg.color[MISC_NORMAL]);
      screen.refresh ();
    }
    
  /* Do reply linking on the newly tossed messages */

  if (msgs)
    {
      win->put (2, 5, "Reply linking...", config.cfg.color[MISC_NORMAL]);
      c = msgbase.get_number_of_messages ();
      screen.refresh ();
      msgbase.link (c - msgs, c);
    }

#if 0
  if (flags ^ 1)
    {
      win->put (2, 6, "Press Enter", config.cfg.color[MISC_HIGHLIGHT]);
      screen.refresh ();
      while (keyboard.get () != 13);
    }
#endif

  chdir (config.cfg.system_path);
  delete win;
  win = NULL;
}
