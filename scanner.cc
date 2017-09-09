/* scanner.cc - scanner for tanstaafl */
/* Copyright 1998-1999 Per Lundberg */

#include "tanstaafl.h"

#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

SCANNER::SCANNER (void)
{
}

SCANNER::~SCANNER (void)
{
}

/* Purpose: Scan the message bases for outgoing mail */
// FIXME: Create BSY files for each packet file created

void SCANNER::play (byte flags)
{
  /* Go through all the records in msghdr.tfl and write the ones flagged as
     Local and not flagged as Sent or Deleted to the appropriate .OUT file */

  FILE *file = NULL, *outfile[10];
  msghdr_type msghdr;
  char tmpstr[256];
  bool packet_opened[10] = { FALSE, FALSE, FALSE, FALSE, FALSE,
			     FALSE, FALSE, FALSE, FALSE, FALSE };
			     
  Window *win;
  dword c = 0, d, n;
  dword last = MAX;
  byte uplink;
  dword *message = NULL;
  dword messages;
  byte mode = 0;

  msgbase.init (config.cfg.msgbase_path);

  sprintf (tmpstr, "%s/messages.tfl", config.cfg.system_path);
  file = fopen (tmpstr, "rb");
  if (file == NULL) /* If this doesn't exist, go kill ourselves */
    {
      if (flags ^ 1)
        {
          if (ui.yesno ("Do a full scan? (This can take some time)") != YES)
            return;
        }
      else
        return;
      mode = 1;
      memset (tmpstr, 0, 256);
      sprintf (tmpstr, "%s/msghdr.tfl", config.cfg.msgbase_path);
      file = fopen (tmpstr, "rb");
      
      if (file == NULL) /* if msghdr.tfl doesn't exist, there won't be any messages to scan */
        {
          return;
        }
      fseek (file, 0, SEEK_END);
      messages = ftell (file) / sizeof (msghdr_type);
      fseek (file, 0, SEEK_SET);
    }
  else
    {
      fseek (file, 0, SEEK_END);
      messages = ftell (file) / 4;
      fseek (file, 0, SEEK_SET);
      message = new dword[messages];
      fread (message, 4, messages, file);
      fclose (file);
      unlink (tmpstr);
    }

  /* Create a BSY file so that no mailer will try to send the file before it's
     finished */

  memset (tmpstr, 0, 256);
  sprintf (tmpstr, "%s/%04x%04x.bsy", config.cfg.outbound_path, config.cfg.uplink[0].net, config.cfg.uplink[0].node);
 
  file = fopen (tmpstr, "w");
  fclose (file);

  win = new Window (screen.x_max / 2 - 30, screen.y_max / 2 - 4, 60, 8, config.cfg.color[MISC_WINDOW], config.cfg.color[MISC_TITLE], "Scanning");
  win->color = config.cfg.color[MISC_NORMAL];
  win->put (2, 1, "Scanning...");
  screen.refresh ();


  for (c = 0; c < messages; c++)
    {
      if (mode == 0)
        {
          msgbase.read_message_header (message[c], &msghdr);
        }
      else
        msgbase.read_message_header (c, &msghdr);

      d = (dword) (((float) c / messages) * 55);
      if (last != d)
	{
	  for (n = last; n < d; n++)
	    win->put (2 + n, 2, ' ', config.cfg.color[MISC_STATUS]);
	  last = (dword) (((float) c / messages) * 55);
	  screen.refresh ();
	}
      
      /* Decide if we should scan out this message */

      if (((msghdr.status_flags & (FLAG_LOCAL | FLAG_SENT)) == FLAG_LOCAL) &&
	  (msghdr.status_flags ^ FLAG_DELETED))
	{
	  packed_msg_type msg;
	  struct tm *tblock;
	  char tmpstr[256];
	  byte *msgtxt = NULL;

	  if (msghdr.folder == 1)
	    sprintf (tmpstr, "To %s (%s) in %s", msghdr.recipient, binToStrAddr (msghdr.recipient_addr), config.area[msghdr.folder]->name);
	  else
	    sprintf (tmpstr, "To %s in %s", msghdr.recipient, config.area[msghdr.folder]->name);
	  win->put (2, 4, tmpstr, config.cfg.color[MISC_NORMAL]);

	  for (d = strlen (tmpstr) + 2; d < 59; d++)
	    win->put (d, 4, ' ', config.cfg.color[MISC_NORMAL]);
	  screen.refresh ();

	  /* Decide which uplink to use for this message. If non-netmail, it's
	     simple; if netmail, a bit trickier. This is since we have only
	     one netmail area and netmail should be routed by matching the
	     destination zone and your uplinks' zones. If no match is found,
	     route via the main uplink */

	  if (msghdr.folder == 1)
	    {
	      for (n = 0; n < 10 && config.cfg.uplink[n].zone; n++)
		if (config.cfg.uplink[n].zone == msghdr.recipient_addr.zone)
		  uplink = n;
	      if (n == 10)
		uplink = 0;
	    }
	  else
	    uplink = config.area[msghdr.folder]->uplink;


	  if (!packet_opened[uplink])
	    {
	      pkthdr_type pkthdr;
	      time_t t;
	      struct tm *tblock;

	      if (config.cfg.uplink[0].zone !=
		  config.cfg.aka[config.area[msghdr.folder]->aka].zone) /* non-primary uplink */
		{
		  sprintf (tmpstr, "%s.%03x", config.cfg.outbound_path, config.cfg.uplink[uplink].zone);
		  mkdir (tmpstr, 0700);
		  sprintf (tmpstr, "%s.%03x/%04x%04x.out", config.cfg.outbound_path,
			   config.cfg.uplink[uplink].zone,
			   config.cfg.uplink[uplink].net,
			   config.cfg.uplink[uplink].node);
		}
	      else
		sprintf (tmpstr, "%s/%04x%04x.out", config.cfg.outbound_path,
			 config.cfg.uplink[uplink].net,
			 config.cfg.uplink[uplink].node);

	      outfile[uplink] = fopen (tmpstr, "r");

	      if (outfile[uplink] != NULL) /* File exists, so append */
		{
		  fclose (outfile[uplink]);
		  outfile[uplink] = fopen (tmpstr, "r+b");
		  fseek (outfile[uplink], -2, SEEK_END);
		  packet_opened[uplink] = TRUE;
		}
	      else
		{
		  outfile[uplink] = fopen (tmpstr, "wb");
		  if (outfile[uplink] == NULL)
		    {
		      ui.infobox ("Error", string ("Could not open ") + string (tmpstr) + string (" for writing"));
		    }
		  else
		    {
		      memset (&pkthdr, 0, sizeof (pkthdr_type));
		      
		      pkthdr.origZone = pkthdr.qOrigZone = msghdr.sender_addr.zone;
		      pkthdr.origNet = msghdr.sender_addr.net;
		      pkthdr.origNode = msghdr.sender_addr.node;
		      pkthdr.origPoint = msghdr.sender_addr.point;
		      
		      pkthdr.destZone = pkthdr.qDestZone = config.cfg.uplink[uplink].zone;
		      pkthdr.destNet = config.cfg.uplink[uplink].net;
		      pkthdr.destNode = config.cfg.uplink[uplink].node;
		      pkthdr.destPoint = config.cfg.uplink[uplink].point;
		      
		      pkthdr.packetVersion = 2;
		      pkthdr.productCodeHi = 0;
		      pkthdr.productCodeLo = 0;
		      pkthdr.productRevMaj = 0;
		      pkthdr.productRevMin = 0;
		      pkthdr.baud = 0;
		      pkthdr.capabilityWord = 2; /* we only support type 2+ */
		      pkthdr.CWValidation = 0x200;
		      
		      time (&t);
		      tblock = localtime (&t);
		      pkthdr.year = tblock->tm_year;
		      pkthdr.month = tblock->tm_mon;
		      pkthdr.day = tblock->tm_mday;
		      
		      pkthdr.hour = tblock->tm_hour;
		      pkthdr.min = tblock->tm_min;
		      pkthdr.sec = tblock->tm_sec;
		      
		      strcpy ((char *) pkthdr.password, config.cfg.password[uplink]);
		      
		      fwrite (&pkthdr, sizeof (pkthdr_type), 1, outfile[uplink]);
		      packet_opened[uplink] = TRUE;
		    }
		}
	    }

	  msg.id = 2;
	  msg.orig_node = msghdr.sender_addr.node;
	  msg.orig_net = msghdr.sender_addr.net;
	  msg.dest_node = msghdr.recipient_addr.node;
	  msg.dest_net = msghdr.recipient_addr.net;
	  msg.cost = 0;
	  if (msghdr.folder == 1) /* If netmail, set the PVT flag */
	    msg.attribute = 1;                                    
	  else
	    msg.attribute = 0;             

	  tblock = gmtime ((time_t *) &msghdr.time_written);
	  sprintf (msg.date, "%02d %3s %02d  %02d:%02d:%02d", tblock->tm_mday, month[tblock->tm_mon], tblock->tm_year % 100, tblock->tm_hour, tblock->tm_min, tblock->tm_sec);
	  fwrite (&msg, sizeof (packed_msg_type), 1, outfile[uplink]);
	  fprintf (outfile[uplink], "%s%c", msghdr.recipient, 0);
	  fprintf (outfile[uplink], "%s%c", msghdr.sender, 0);
	  fprintf (outfile[uplink], "%s%c", msghdr.subject, 0);

	  if (msghdr.folder != 1)
	    {
	      fprintf (outfile[uplink], "AREA:%s\r", config.area[msghdr.folder]->name);
	    }

          msgbase.read_message_text (c, msghdr);
	  fwrite (msgbase.msgtxt, msghdr.txt_size, 1, outfile[uplink]);
	  msghdr.status_flags |= FLAG_SENT;

          if (mode == 0)
            msgbase.write_message_header (message[c], msghdr);
          else
            msgbase.write_message_header (c, msghdr);
	  fprintf (outfile[uplink], "%c", 0);
	}
    }

  if (last == MAX) last = 0;

  for (n = last; n <= 55; n++)
    win->put (2 + n, 2, ' ', config.cfg.color[MISC_STATUS]);

  for (n = 0; n < 10; n++)
    if (packet_opened[n])
      {
	fprintf (outfile[n], "%c%c", 0, 0);
	fclose (outfile[n]);
      }

  /* Remove the BSY file */

  memset (tmpstr, 0, 256);
  sprintf (tmpstr, "%s/%04x%04x.bsy", config.cfg.outbound_path, config.cfg.uplink[0].net, config.cfg.uplink[0].node);
 
  unlink (tmpstr);

#if 0
  if (flags ^ 1)
    {
      win->put (2, 6, "Press Enter.", config.cfg.color[MISC_HIGHLIGHT]);
      screen.refresh ();
      keyboard.get ();
    }
#endif
  delete win;
}

