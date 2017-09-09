/* reader.cc - main reader part of tanstaafl */
/* Copyright 1998-1999 Per Lundberg */
/* Copyright 1999 Bartek Kania */
/* Copyright 1999 Johannes Lundberg */

/* This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the

   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include "tanstaafl.h"
#include "crc32.h"
#include "build_number.h"

#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>

extern uchar line_char[12];

READER::READER (void)
{
  msghdr = new msghdr_type;
  last_message = last_area = last_flags = MAX; /* -1 */
  choice = 1;
  add = 0;
}

READER::~READER (void)
{
  delete msghdr;
}

/* Show the time in the bottom of the screen */

void show_time (void)
{
  time_t t;
  tm *tblock;
  char tmpstr[9];

  time (&t);
  tblock = localtime (&t);
  sprintf (tmpstr, "³ %02d:%02d ", tblock->tm_hour, tblock->tm_min);
  screen.put (screen.x_max - 7, screen.y_max, tmpstr, config.cfg.color[MISC_STATUS]);
}

/* Used by list_messages */

void READER::show_line (dword choice, dword add)
{
  byte c;
  char tmpstr[256];
  char subject[256];
  msghdr_type msghdr;
  struct tm *tblock;

  for (c = 0; c < screen.y_max - 2 && add + c < msgbase.messages; c++)
  {
    msgbase.read_message_header (msgbase.msg[add + c], &msghdr);

    tblock = gmtime ((const time_t *) &msghdr.time_written);

    if (strlen ((char *) msghdr.sender) > 19)
      strcpy ((char *) msghdr.sender + 16, "...");

    if (strlen ((char *) msghdr.recipient) > 19)
      strcpy ((char *) msghdr.recipient + 16, "...");

    if (strlen ((char *) msghdr.subject) > (screen.x_max - 59))
    {
//      msghdr.subject[screen.x_max - 59] = 0;
      strcpy (subject, (char *) msghdr.subject);
      strcpy (subject + screen.x_max - 62, "...");
    }
    else
    {
      memset (subject, 0x20, 256);
      strncpy (subject, (char *) msghdr.subject,
	       strlen ((char *) msghdr.subject));
      subject[screen.x_max - 59] = 0;
    }

    sprintf (tmpstr, " %c %5d %-19s %-19s %s %3s %02u ",
	     ((msghdr.status_flags & FLAG_SEEN) == FLAG_SEEN) ? ' ' : 'N',
	     add + c + 1,
	     msghdr.sender, msghdr.recipient, subject,
	     month[tblock->tm_mon], tblock->tm_mday);
    if (c == choice)
    {
      screen.put (2, c + 2, tmpstr, config.cfg.color[MISC_CHOSEN]);
      screen.gotoxy (2, c + 2);
    }
    else
    {
      if (!strcasecmp ((char *) msghdr.sender, config.cfg.user_name) || 
	  !strcasecmp ((char *) msghdr.recipient, config.cfg.user_name))
	screen.put (2, c + 2, tmpstr, config.cfg.color[MISC_HIGHLIGHT]);
      else
	screen.put (2, c + 2, tmpstr, config.cfg.color[MISC_NORMAL]);
    }
  }
  screen.refresh ();
}

/* List the messages in the current area */

dword READER::list_messages (dword message, word area_number)
{
  byte c;
  char done = FALSE;
  char tmpstr[256];
  dword choice = screen.y_max / 2 - 1, add = message;

  if (add > choice)
    add -= choice;
  else if (choice > message)
  {
    choice = message;
    add = 0;
  }
  else
    add = 0;

  if (msgbase.messages < choice)
    /* less thean screen.y_max / 2 - 1 messages in the area */
  {
    add = 0;
    choice = msgbase.messages - 1;
  }

  screen.clear ();
  screen.window (1, 1, screen.x_max, screen.y_max, config.cfg.color[MISC_WINDOW]);
  sprintf (tmpstr, "%c Area: %s (%s) %c", line_char[7], 
	   config.area[area_number]->name, config.area[area_number]->comment,
	   line_char[6]);
  screen.center (1, tmpstr, config.cfg.color[MISC_TITLE]);
  
  show_line (choice, add);

  do
  {
    c = keyboard.get ();
    switch (c)
    {
      case 0: /* Extended key */
	c = keyboard.get ();
	switch (c)
	{
	  case 71: /* Home */
	    choice = add = 0;
	    show_line (choice, add);
	    break;
	  case 72: /* Up arrow */
	    if (choice) 
	      choice--;
	    else if (add)
	      add--;
	    show_line (choice, add);
	    break;
	  case 73: /* Page Up */
	    if (add > screen.y_max - 2)
	      add -= screen.y_max - 2;
	    else
	      choice = add = 0;
	    show_line (choice, add);
	    break;
	  case 79: /* End */
	    choice = screen.y_max - 3;
	    if (choice > msgbase.messages - 1)
	      choice = msgbase.messages - 1;
	    add = msgbase.messages - choice - 1;
	    show_line (choice, add);
	    break;
	  case 80: /* Down arrow */
	    if (choice < screen.y_max - 3)
	    {
	      if (choice + add < msgbase.messages - 1) 
		choice++;
	    }
	    else
	      if (choice + add < msgbase.messages - 1) 
		add++;
	    show_line (choice, add);
	    break;
	  case 81: /* Page Down */
	    if (choice + add < msgbase.messages - screen.y_max - 2)
	      add += screen.y_max - 2;
	    else
	    {
	      choice = screen.y_max - 3;
	      if (choice > msgbase.messages - 1)
		choice = msgbase.messages - 1;
	      add = msgbase.messages - choice - 1;
	    }
	    show_line (choice, add);
	    break;
	}
	break;
      case 13: /* Enter */
	done = TRUE;
	break;
      case 27: /* Escape */
	done = ABORT;
	break;
    }
  } while (!done);
  screen.clear ();
  if (done == ABORT)
    return message;
  else
    return choice + add;
}

/* Show the current message on the screen, starting from start_line */

void READER::show_message (dword message, word area_number, word start_line,
			   byte flags)
{
  char tmpstr[256];
  dword c = 0, d = 0;
  uchar x = 0;
  struct tm *tblock;
  float f;
  byte color;
  byte header_size;

  /* Decide how big the header will be */

  if (config.cfg.separating_line)
  {
    header_size = 6;
  }
  else
    header_size = 5;

  if (last_area != area_number || last_message != message  || last_flags != flags) 
  {
    rows = 0;
    if (msgbase.read_message (message, msghdr) == ABORT)
      return;
      
    msglen = strlen ((char *) msgbase.msgtxt);

    /* Mark the message as seen, if it was not already marked */

    if ((msghdr->status_flags & FLAG_SEEN) == 0)
    {
      msghdr->status_flags |= FLAG_SEEN;
      msgbase.write_message_header (msgbase.msg[message], *msghdr);
    }

    /* Replace all carriage returns with zeroes, for easy parsing */

    for (d = 0; d < msghdr->txt_size; d++)
      if (msgbase.msgtxt[d] == 0x0D) 
      {
	msgbase.msgtxt[d] = 0;
	rows++;
      }

    /* If a line is longer than the screen width, it must be split. Let's do that. */
    // FIXME: this bugs occasionally

    x = d = 0;
    while (d < msghdr->txt_size)
    {
      x++; d++;

      if (x > screen.x_max && msgbase.msgtxt[d])
      {
	x = 0;
	while (msgbase.msgtxt[d - x] != ' ' && x < screen.x_max)
	  x++;
	      
	if (x == screen.x_max) /* we could not split the line, so we don't */
	  x = 0;
	else
	  msgbase.msgtxt[d - x] = 0;

	rows++;
      }
      else
	if (!msgbase.msgtxt[d]) /* end of a line */
	{
	  x = 0;
	}
    }

    d = 0;
      
    while (d < msghdr->txt_size)
    {
      if (is_kludge ((char *) msgbase.msgtxt + d))
	if ((flags & 1) != 1) /* don't show kludges */
	  rows--;
      d += strlen ((char *) msgbase.msgtxt + d) + 1;
    }

    if (rows > screen.y_max - header_size) 
      rows -= screen.y_max - header_size;
    else
      rows = 0;

    last_message = message;
    last_area = area_number;
    last_flags = flags;
  }

  /* Print the message header */
    
  screen.put (1, 1, "Date : ", config.cfg.color[READER_HEADER]);
  screen.put (1, 2, "From : ", config.cfg.color[READER_HEADER]);
  screen.put (1, 3, "To   : ", config.cfg.color[READER_HEADER]);
  screen.put (1, 4, "Subj : ", config.cfg.color[READER_HEADER]);
  
  tblock = gmtime ((const time_t *) &msghdr->time_written);

  sprintf (tmpstr, "%s %s %02d, %02d:%02d", month[tblock->tm_mon], 
	   weekday[tblock->tm_wday], tblock->tm_mday, tblock->tm_hour, 
	   tblock->tm_min);
  screen.put (8, 1, tmpstr, config.cfg.color[READER_HEADER]);
  screen.put (8, 2, msghdr->sender, config.cfg.color[READER_HEADER]);
  for (c = 8 + strlen ((char *) msghdr->sender); c < 51; c++)
    screen.put (c, 2, ' ', config.cfg.color[READER_HEADER]);
  sprintf (tmpstr, "%s", msghdr->recipient);
  if (!strcasecmp ((char *) msghdr->recipient, config.cfg.user_name))
    screen.put (8, 3, tmpstr, config.cfg.color[READER_PERSONAL]);
  else
    screen.put (8, 3, tmpstr, config.cfg.color[READER_HEADER]);
  for (c = 8 + strlen (tmpstr); c < 51; c++)
    screen.put (c, 3, ' ', config.cfg.color[READER_HEADER]);
  screen.put (8, 4, msghdr->subject, config.cfg.color[READER_HEADER]);
  for (c = 8 + strlen ((char *) msghdr->subject); c < screen.x_max + 1; c++)
    screen.put (c, 4, ' ', config.cfg.color[READER_HEADER]);

  /* Always print the sender adress */

  sprintf (tmpstr, ", %u:%u/%u.%u", msghdr->sender_addr.zone, 
	   msghdr->sender_addr.net, msghdr->sender_addr.node, 
	   msghdr->sender_addr.point);
  screen.put (8 + strlen ((char *) msghdr->sender), 2, tmpstr, config.cfg.color[READER_HEADER]);
  for (x = 8 + strlen ((char *) msghdr->sender) + strlen (tmpstr); x < screen.x_max + 1; x++)
    screen.put (x, 2, 32, config.cfg.color[READER_HEADER]);

  /* If netmail, also print the receiver adress */

  if (area_number == 1)
  {
    sprintf (tmpstr, ", %u:%u/%u.%u", msghdr->recipient_addr.zone, 
	     msghdr->recipient_addr.net, msghdr->recipient_addr.node, 
	     msghdr->recipient_addr.point);
    screen.put (8 + strlen ((char *) msghdr->recipient), 3, tmpstr, config.cfg.color[READER_HEADER]);
    for (x = 8 + strlen ((char *) msghdr->recipient) + strlen (tmpstr); x < screen.x_max + 1; x++)
      screen.put (x, 3, 32, config.cfg.color[READER_HEADER]);
  }
  else
    for (x = 50; x < screen.x_max + 1; x++)
      screen.put (x, 3, 32, config.cfg.color[READER_HEADER]);

  /* Are we in reply linking mode? If so, indicate it */

  c = 25;
  if (flags & (1 << 2)) /* reply linking mode; indicate this */
  {
    sprintf (tmpstr, "     Reply linking mode ");
    screen.put (25, 1, tmpstr, config.cfg.color[READER_HEADER]);
    c += strlen (tmpstr);
  }

  /* Show message attributes */

  strcpy (tmpstr, "");
  if (msghdr->status_flags & FLAG_PRIVATE) /* Private */
    strcat (tmpstr, "prv ");
  if ((msghdr->status_flags & FLAG_LOCAL) && (msghdr->status_flags & FLAG_SENT))
    strcat (tmpstr, "snt ");
  if (area_number == 1 && !(msghdr->status_flags & FLAG_LOCAL) &&
      (msghdr->status_flags & FLAG_READ)) /* Received */
    strcat (tmpstr, "rcv ");
  if (msghdr->status_flags & FLAG_LOCAL)  /* Local */
    strcat (tmpstr, "loc ");               
  if (msghdr->status_flags & FLAG_ATTACH) /* Fileattach */
    strcat (tmpstr, "fat ");
  if (msghdr->status_flags & FLAG_CRASH)  /* Crash */
    strcat (tmpstr, "crsh ");
  if (msghdr->status_flags & FLAG_HOLD)   /* Hold */
    strcat (tmpstr, "hld ");

  for (;c < screen.x_max - strlen (tmpstr) + 1; c++)
    screen.put (c, 1, ' ', config.cfg.color[READER_HEADER]);

  screen.put (screen.x_max - strlen (tmpstr) + 1, 1, tmpstr, config.cfg.color[READER_HEADER]);

  /* Indicate whether there are comments to this message or not */

  if (msghdr->reply_mother != MAX)
    screen.put (screen.x_max - 3, 2, 30, config.cfg.color[READER_HEADER]); /* Up */
  if (msghdr->reply_child != MAX)
    screen.put (screen.x_max - 3, 4, 31, config.cfg.color[READER_HEADER]); /* Down */
  if (msghdr->reply_next != MAX)
    screen.put (screen.x_max - 2, 3, 16, config.cfg.color[READER_HEADER]); /* Right */
  if (msghdr->reply_prev != MAX)
    screen.put (screen.x_max - 4, 3, 17, config.cfg.color[READER_HEADER]); /* Left */

  /* Write a nice separating line */

  if (config.cfg.separating_line)
  {
    screen.color = config.cfg.color[READER_HEADER];
    screen.hline (5);
  }

  c = d = 0;
  while (c < start_line)
  {
    if ((flags & 1) != 1) /* don't show kludges */
      while (is_kludge ((char *) msgbase.msgtxt + d) && d < msghdr->txt_size) 
	d += strlen ((char *) msgbase.msgtxt + d) + 1;
    if (!msgbase.msgtxt[d++])
      c++;
  }

  /* Show a status line */

  screen.put (1, screen.y_max, " Area: ", config.cfg.color[MISC_STATUS]);
  screen.put (8, screen.y_max, config.area[area_number]->comment, config.cfg.color[MISC_STATUS]);

  show_time ();

  if (rows)
    f = (((float) start_line + screen.x_max - header_size) / (rows + screen.x_max - header_size)) * 100;
  else
    f = 100;
  if (f > 100) f = 100;

  sprintf (tmpstr, "%3.0f%% of message %u / %d ", f, message + 1, msgbase.messages);
  screen.put (screen.x_max - 7 - strlen (tmpstr), screen.y_max, tmpstr, config.cfg.color[MISC_STATUS]);

  for (c = 8 + strlen (config.area[area_number]->comment); c < screen.x_max - strlen (tmpstr) - 7; c++)
    screen.put (c, screen.y_max, ' ', config.cfg.color[MISC_STATUS]);

  /* And finally, display the message */

  for (c = 0; c < screen.y_max - header_size; c++)
  {
    if (d < msghdr->txt_size)
    {
      /* Kludge viewing mode. */

      if ((flags & 1) != 1)
	while (is_kludge ((char *) msgbase.msgtxt + d) &&
               d < msghdr->txt_size) 
	  d += strlen ((char *) msgbase.msgtxt + d) + 1;
      strcpy (tmpstr, (char *) msgbase.msgtxt + d);

      /* ROT13 mode. */

      if ((flags & 2) == 2)
      {
	for (x = 0; tmpstr[x] != '\0'; x++)
	{
	  if (tolower (tmpstr[x]) >= 'a' && tolower (tmpstr[x]) <= 'm')
          {
            tmpstr[x] += 13;
          }
	  else if (tolower (tmpstr[x]) >= 'n' && tolower (tmpstr[x]) <= 'z')
          {
	    tmpstr[x] -= 13;
          }
	}
      }

      for (x = 0; tmpstr[x] != '\0'; x++)
      {
        if ((byte) tmpstr[x] == 0x8D)
        {
          tmpstr[x] = ' ';
        }
      }

      d += strlen (tmpstr) + 1;
      if (is_kludge (tmpstr)) 
      {
	color = config.cfg.color[READER_KLUDGE];
      }
      else if (is_origin (tmpstr))
      {
	color = config.cfg.color[READER_ORIGIN];
      }
      else if (is_quote (tmpstr))
      {
	color = config.cfg.color[READER_QUOTE];
      }
      else
      {
	color = config.cfg.color[READER_NORMAL];
      }
      screen.put (1, header_size + c, tmpstr, color);
    }
    else
    {
      strcpy (tmpstr, "");
    }

    for (x = strlen (tmpstr) + 1; x < screen.x_max + 1; x++)
    {
      screen.put (x, header_size + c, ' ', config.cfg.color[READER_NORMAL]);
    }
  }
  screen.refresh ();
}

/* Open the selected area, beginning with the lastread message */

void READER::open_area (word area_number)
{
  uchar c;
  uchar flag = 0; /* bit 0 = show kludges
		     bit 1 = ROT13 mode 
		     bit 2 = reply linking mode */
  dword row = 0;
  dword d;
  bool done = FALSE;
  bool mode = 0; /* 0 = normal reading mode, 1 = reply linking mode. If in
		    reply linking mode, only reply linking keys work, and
		    escape returns to the original message. */
  dword earlier_message;
  word earlier_area;

  last_message = last_area = last_flags = MAX; /* -1 */

  msgbase.init (string (config.cfg.msgbase_path));
  if (msgbase.open_area (area_number, &curmsg) == ABORT) return;

  if (msgbase.messages)
  {
    screen.clear ();
    show_message (curmsg, area_number, 0, flag);
  }
  else
  {
    screen.clear ((config.cfg.color[MISC_NORMAL] << 8) + ' ');
    screen.center (screen.y_max / 2, "Area empty. Press insert to enter a new message", config.cfg.color[MISC_NORMAL]);
    screen.refresh ();
  } 

  do
  {
    c = keyboard.get ();
    if (mode) /* reply linking mode; only respond to certain keys */
    {
      switch (tolower (c))
      {
	case 0:
	  c = keyboard.get ();
	  switch (c)
	  {
	    case 72:
	      if (*(keyboard.shift_state) & CONTROL) /* Control-Up */
	      {
		d = msgbase.linear_to_logic (msghdr->reply_mother);
		if (d == MAX) /* another area or non-existing message */
		{
		  d = msgbase.area_number (msghdr->reply_mother);
		  if (d != MAX) /* non-existing message */
		  {
		    area_number = d;
		    last_message = last_area = last_flags = MAX; /* -1 */
		    msgbase.open (d);
		    curmsg = msgbase.linear_to_logic (msghdr->reply_mother);
		  }
		}
		else
		  curmsg = d;
		row = 0;
		show_message (curmsg, area_number, row, flag);
	      }
	      break;
	    case 80:
	      if (*(keyboard.shift_state) & CONTROL) /* Control-Down */
	      {
		d = msgbase.linear_to_logic (msghdr->reply_child);
		if (d == MAX) /* another area or non-existing message */
		{
		  d = msgbase.area_number (msghdr->reply_child);
		  if (d != MAX) /* non-existing message */
		  {
		    area_number = d;
		    last_message = last_area = last_flags = MAX; /* -1 */
		    msgbase.open (d);
		    curmsg = msgbase.linear_to_logic (msghdr->reply_child);
		  }
		}
		else
		  curmsg = d;
		row = 0;
		show_message (curmsg, area_number, row, flag);
	      }
	      break;
	    case 75:
	      if (*(keyboard.shift_state) & CONTROL) /* Ctrl-Left */
	      {
		d = msgbase.linear_to_logic (msghdr->reply_prev);
		if (d == MAX) /* another area or non-existing message */
		{
		  d = msgbase.area_number (msghdr->reply_prev);
		  if (d != MAX) /* non-existing message */
		  {
		    area_number = d;
		    msgbase.open (d);
		    last_message = last_area = last_flags = MAX; /* -1 */
		    curmsg = msgbase.linear_to_logic (msghdr->reply_prev);
		  }
		}
		else
		  curmsg = d;
		row = 0;
		show_message (curmsg, area_number, row, flag);
	      }
	      break;
	    case 77:
	      if (*(keyboard.shift_state) & CONTROL) /* Ctrl-Right */
	      {
		d = msgbase.linear_to_logic (msghdr->reply_next);
		if (d == MAX) /* another area or non-existing message */
		{
		  d = msgbase.area_number (msghdr->reply_next);
		  if (d != MAX) /* non-existing message */
		  {
		    area_number = d;
		    msgbase.open (d);
		    last_message = last_area = last_flags = MAX; /* -1 */
		    curmsg = msgbase.linear_to_logic (msghdr->reply_next);
		  }
		}
		else
		  curmsg = d;
		row = 0;
		show_message (curmsg, area_number, row, flag);
	      }
	      break;
	    default:
	      break;
	  }
	  break;
	case 27: /* Escape, turn off reply linking mode */
	  mode = 0;
	  flag ^= (2 << 1);
	  curmsg = earlier_message;
 	  area_number = earlier_area;
	  msgbase.open (area_number);
	  show_message (curmsg, area_number, row, flag);
	  break;
	default:
	  break;
      }
	      
    }
    else
      switch (tolower (c))
      {
	case 0: /* Extended keys */
	  c = keyboard.get ();
	  switch (c)
	  {
	    case 22: /* Alt-U */
	      msghdr->status_flags &= (MAX - 0x10);
	      msgbase.write_message_header (msgbase.msg[curmsg], *msghdr);
	      show_message (curmsg, area_number, row, flag);
	      break;
	    case 31: /* Alt-S */
	      search (area_number, flag);
	      break;     
	    case 35: /* Alt-H, edit header */
	      if (msgbase.messages && ((msghdr->status_flags & 1) == 1))
	      {
		editor.edit_header (msghdr);
		msgbase.write_message_header (msgbase.msg[curmsg], *msghdr);
		show_message (curmsg, area_number, row, flag);
	      }
	      break;
	    case 49: /* Alt-N, netmail-reply */
	      if (msgbase.messages)
	      {
		write_message (*msghdr, msgbase.msgtxt, 1, REPLY);
		show_message (curmsg, area_number, row, flag);
	      }
	      break;
	    case 59: /* F1, help */
	      help ();
	      break;
	    case 71: /* Home, beginning of message */
	      row = 0;
	      show_message (curmsg, area_number, row, flag);
	      break;
	    case 79: /* End, end of message */
	      row = rows;
	      show_message (curmsg, area_number, row, flag);
	      break;
	    case 117: /* Ctrl-Home, beginning of area */
	      if (msgbase.messages)
	      {
		row = 0;
		curmsg = 0;
		show_message (curmsg, area_number, row, flag);
	      }
	      break;
	    case 119: /* Ctrl-End, end of area */
	      if (msgbase.messages)
	      {
		curmsg = msgbase.messages - 1;
		row = 0;
		show_message (curmsg, area_number, row, flag);
	      }
	      break;  
	    case 72:
	      if (*(keyboard.shift_state) & CONTROL) /* Control-Up */
	      {
		earlier_message = curmsg;
		earlier_area = area_number;

		d = msgbase.linear_to_logic (msghdr->reply_mother);
		if (d == MAX) /* another area or non-existing message */
		{
		  d = msgbase.area_number (msghdr->reply_mother);
		  if (d != MAX) /* non-existing message */
		  {
		    area_number = d;
		    last_message = last_area = last_flags = MAX; /* -1 */
		    msgbase.open (d);
		    curmsg = msgbase.linear_to_logic (msghdr->reply_mother);
		    mode = 1;
		    flag ^= (1 << 2);
		  }
		}
		else
		{
		  curmsg = d;
		  mode = 1;
		  flag ^= (1 << 2);
		}
		row = 0;
		show_message (curmsg, area_number, row, flag);
	      }
	      else /* Up Arrow */
		if (msgbase.messages)
		{
		  if (row)
		  {
		    row--;
		    show_message (curmsg, area_number, row, flag);
		  }
		}
	      break;
	    case 80:
	      if (*(keyboard.shift_state) & CONTROL) /* Control-Down */
	      {
		earlier_message = curmsg;
		earlier_area = area_number;
		
		d = msgbase.linear_to_logic (msghdr->reply_child);
		if (d == MAX) /* another area or non-existing message */
		{
		  d = msgbase.area_number (msghdr->reply_child);
		  if (d != MAX) /* non-existing message */
		  {
		    area_number = d;
		    last_message = last_area = last_flags = MAX; /* -1 */
		    msgbase.open (d);
		    curmsg = msgbase.linear_to_logic (msghdr->reply_child);
		    mode = 1;
		    flag ^= (1 << 2);
		  }
		}
		else
		{
		  curmsg = d;
		  mode = 1;
		  flag ^= (1 << 2);
		}
		row = 0;
		show_message (curmsg, area_number, row, flag);
	      }
	      else /* Down Arrow */
		if (msgbase.messages)
		{
		  if (row < rows)
		    row++;
		  show_message (curmsg, area_number, row, flag);
		}
	      break;
	    case 73: /* Page Up */
	      if (row > screen.y_max - 6)
		row -= screen.y_max - 6;
	      else
		row = 0;
	      show_message (curmsg, area_number, row, flag);
	      break;
	    case 81: /* Page Down */
	      row += screen.y_max - 6;
	      if (row > rows) row = rows;
	      show_message (curmsg, area_number, row, flag);
	      break;
	    case 75:
	      if (*(keyboard.shift_state) & CONTROL) /* Ctrl-Left */
	      {
		earlier_message = curmsg;
		earlier_area = area_number;

		d = msgbase.linear_to_logic (msghdr->reply_prev);
		if (d == MAX) /* another area or non-existing message */
		{
		  d = msgbase.area_number (msghdr->reply_prev);
		  if (d != MAX) /* non-existing message */
		  {
		    area_number = d;
		    msgbase.open (d);
		    last_message = last_area = last_flags = MAX; /* -1 */
		    curmsg = msgbase.linear_to_logic (msghdr->reply_prev);
		    mode = 1;
		    flag ^= (1 << 2);
		  }
		}
		else
		{
		  curmsg = d;
		  mode = 1;
		  flag ^= (1 << 2);
		}
		row = 0;
		show_message (curmsg, area_number, row, flag);
	      }
	      else /* Left Arrow */
	      {
		if (curmsg && msgbase.messages)
		{
		  curmsg--;
		  row = 0;
		  show_message (curmsg, area_number, row, flag);
		}
	      }
	      break;
	    case 77:
	      if (*(keyboard.shift_state) & CONTROL) /* Ctrl-Right */
	      {
		earlier_message = curmsg;
		earlier_area = area_number;

		d = msgbase.linear_to_logic (msghdr->reply_next);
		if (d == MAX) /* another area or non-existing message */
		{
		  d = msgbase.area_number (msghdr->reply_next);
		  if (d != MAX) /* non-existing message */
		  {
		    area_number = d;
		    msgbase.open (d);
		    last_message = last_area = last_flags = MAX; /* -1 */
		    curmsg = msgbase.linear_to_logic (msghdr->reply_next);
		    mode = 1;
		    flag ^= (1 << 2);
		  }
		}
		else
		{
		  curmsg = d;
		  mode = 1;
		  flag ^= (1 << 2);
		}
		row = 0;
		show_message (curmsg, area_number, row, flag);
	      }
	      else /* Right Arrow */
	      {
		if (curmsg < msgbase.messages - 1 && msgbase.messages) 
		{
		  curmsg++;
		  row = 0 ;
		  show_message (curmsg, area_number, row, flag);
		}
		else /* Jump to the next area with unread messages */
		{
		  if (config.cfg.jump_to_next)
		  {
                    /* FIXME: Highly unoptimized. We should use the
                       index for this. */
                    
		    msgbase.close_area ();
		    do
		    {
		      msgbase.open_area (area_number++, &curmsg);
		    } while ((curmsg == msgbase.messages - 1 ||
                              !msgbase.messages) &&
                             area_number < config.area_header.areas);

		    if (area_number == config.area_header.areas)
		    {
		      done = TRUE;
		    }
		    else
		    {
		      area_number--;
		      row = 0;
		      show_message (curmsg, area_number, row, flag);
		    }
		  }
		}
	      }
	      break;
	    case 82: /* Insert, write a new message */
            {
	      write_message (*msghdr, msgbase.msgtxt, area_number, ORIGINAL);
	      if (msgbase.messages)
              {
		show_message (curmsg, area_number, row, flag);
              }
	      break;
            }
	    case 83: /* Delete */ // FIXME: make this more stable
#if 0
	      if (msgbase.messages)
	      {
		msgbase.delete_message (msgbase.msg[curmsg]);
		msgbase.close_area ();
		msgbase.open_area (area_number, &curmsg);
		if (curmsg > msgbase.messages && curmsg)
		{
		  curmsg--;
		  show_message (curmsg, area_number, row, flag);
		}
	      }
#endif
	      break;
	    case 115:
	      d = msgbase.linear_to_logic (msghdr->reply_prev);
	      earlier_message = curmsg;
	      earlier_area = area_number;

	      if (d == MAX) /* another area or non-existing message */
	      {
		d = msgbase.area_number (msghdr->reply_prev);
		if (d != MAX) /* non-existing message */
		{
		  area_number = d;
		  msgbase.open (d);
		  last_message = last_area = last_flags = MAX; /* -1 */
		  curmsg = msgbase.linear_to_logic (msghdr->reply_prev);
		  mode = 1;
		  flag ^= (1 << 2);
		}
	      }
	      else
	      {
		curmsg = d;
		mode = 1;
		flag ^= (1 << 2);
	      }
	      row = 0;
	      show_message (curmsg, area_number, row, flag);
	      break;
	    case 116: /* Ctrl-Right */
	      d = msgbase.linear_to_logic (msghdr->reply_next);
	      earlier_message = curmsg;
	      earlier_area = area_number;

	      if (d == MAX) /* another area or non-existing message */
	      {
		d = msgbase.area_number (msghdr->reply_next);
		if (d != MAX) /* non-existing message */
		{
		  area_number = d;
		  msgbase.open (d);
		  last_message = last_area = last_flags = MAX; /* -1 */
		  curmsg = msgbase.linear_to_logic (msghdr->reply_next);
		  mode = 1;
		  flag ^= (1 << 2);
		}
	      }
	      else
	      {
		curmsg = d;
		mode = 1;
		flag ^= (1 << 2);
	      }
	      row = 0;
	      show_message (curmsg, area_number, row, flag);
	      break;

#ifdef DEBUG
	    default:
	      printf ("%d\n", c);
	      break;
#endif
	  }
	  break;
	case 6: /* Ctrl-F, forward the current message to another area */
	  if (msgbase.messages)
	  {
	    write_message (*msghdr, msgbase.msgtxt, choose_area (), FORWARD);
	    show_message (curmsg, area_number, row, flag);
	  }
	  break;
	case 13: /* Enter, go to next message */
	  if (curmsg < msgbase.messages - 1 && msgbase.messages) 
	  {
	    curmsg++;
	    row = 0 ;
	    show_message (curmsg, area_number, row, flag);
	  }
	  break;
	case 17: /* Ctrl-R, reply to sender */
	  if (msgbase.messages)
	  {	    
	    write_message (*msghdr, msgbase.msgtxt, config.area[area_number]->reply_area, REPLY_TO_RECIPIENT);
	    show_message (curmsg, area_number, row, flag);
	  }
	  break;
	case 27: /* Escape, quit reader mode */
	  done = TRUE;
	  break;
	case 'g': /* Goto a message number */
	{
	  Window *win;
	  char tmpstr[10];
	  memset (tmpstr, 0, 10);
	  win = new Window (0, 0, 20, 3, config.cfg.color[MISC_WINDOW], config.cfg.color[MISC_TITLE], "Enter number");
	  screen.refresh ();
	  win->get_line (2, 1, tmpstr, 8, 1);
	  delete win;
	  if (atol (tmpstr) <= msgbase.messages && atol (tmpstr))
	  {
	    curmsg = atol (tmpstr) - 1;
	    show_message (curmsg, area_number, row, flag);
	  }
	  break;
	}
	case 'i': /* Show info about the current message */
	{
	  Window *win;
	  char tmpstr[256];
	  win = new Window (0, 0, 25, 7, config.cfg.color[MISC_WINDOW], config.cfg.color[MISC_TITLE], "Message Information");
	  sprintf (tmpstr, "Message: %u", curmsg);
	  win->put (tmpstr);
	  sprintf (tmpstr, "Linear: %u", msgbase.msg[curmsg]);
	  win->put (tmpstr);
	  sprintf (tmpstr, "MSGID CRC: 0x%08X", msghdr->msgid_CRC);
	  win->put (tmpstr);
	  sprintf (tmpstr, "REPLY CRC: 0x%08X", msghdr->reply_CRC);
	  win->put (tmpstr);
	  sprintf (tmpstr, "Flags: 0x%08X", msghdr->status_flags);
	  win->put (tmpstr);
	  screen.refresh ();
	  keyboard.get ();
	  delete win;
	  break;
	}
	case 'k': /* Toggle kludge view */
	  flag ^= 1;
	  show_message (curmsg, area_number, row, flag);
	  break;
	case 'l': /* List messages */
	  curmsg = list_messages (curmsg, area_number);
	  show_message (curmsg, area_number, row, flag);
	  break;
	case 'n': /* Reply in other area */
	  if (msgbase.messages)
	  {
	    write_message (*msghdr, msgbase.msgtxt, choose_area (), REPLY);
	    show_message (curmsg, area_number, row, flag);
	  }
	  break;
	case 'o': /* ROT13-encode/decode */
	  flag ^= 2;
	  show_message (curmsg, area_number, row, flag);
	  break;
	case 'r': /* Reply to message */
	  if (msgbase.messages)
	  {	    
	    write_message (*msghdr, msgbase.msgtxt, config.area[area_number]->reply_area, REPLY);
	    show_message (curmsg, area_number, row, flag);
	  }
	  break;
	case 'e': /* Export message */
	  if (msgbase.messages)
	    export_message (curmsg, area_number, flag);
	  break;
#ifdef DEBUG
	default:
	  printf ("%d\n", c);
	  break;
#endif
      }
  } while (!done);
  msgbase.close_area ();
}

/* Used by the area selector */

void READER::refresh_screen (word choice, word add)
{
  byte c, c2;
  char tmpstr[256];
  char name[30];
  char comment[256];

  for (c = 1; c < screen.y_max && c + add < config.area_header.areas; c++)
  {
    strcpy (name, config.area[add + c]->name);
    if (strlen (config.area[add + c]->name) > 21)
    {
      strcpy (name + 18, "...");
    }
    
    if (strlen (config.area[add + c]->comment) > (screen.x_max - 43))
    {
      strcpy (comment, config.area[add + c]->comment);
      strcpy (comment + screen.x_max - 43, "...");
    }
    else
    {
      memset (comment, 0x20, 256);
      strncpy (comment, config.area[add + c]->comment,
	      strlen (config.area[add + c]->comment));
      comment[screen.x_max - 40] = 0;
    }

    if ((total[add + c] != last[add + c] + 1) && total[add + c] > 0)
    {
      sprintf (tmpstr, "%-21s %s   %5d NEW %5d",
               name,
               comment,
               total[add + c] - last[add + c] - 1,
               total[add + c]);
      if (c == choice) 
      {
        screen.color = config.cfg.color[MISC_STATUS];
        screen.gotoxy (1, c + 2);
      }
      else
        screen.color = config.cfg.color[EDITOR_QUOTE];
    }
    else
    {
      sprintf (tmpstr, "%-21s %s             %5d",
               name,
               comment,
               total[add + c]);
      if (c == choice) 
      {
        screen.color = config.cfg.color[MISC_STATUS];
        screen.gotoxy (1, c + 2);
      }
      else
        screen.color = config.cfg.color[EDITOR_NORMAL];
    }      
    screen.put (1, 1 + c, tmpstr);
  }
  screen.refresh ();
}

/* Used by the area selector */

void READER::refresh_line (word choice, word c, word add)
{
  byte c2;
  char tmpstr[256];
  char name[30];
  char comment[256];
  
  if (c + add < config.area_header.areas)
  {

    strcpy (name, config.area[add + c]->name);
    if (strlen (config.area[add + c]->name) > 21)
    {
      strcpy (name + 18, "...");
    }
    
    if (strlen (config.area[add + c]->comment) > (screen.x_max - 43))
    {
      strcpy (comment, config.area[add + c]->comment);
      strcpy (comment + screen.x_max - 43, "...");
    }
    else
    {
      memset (comment, 0x20, 256);
      strncpy (comment, config.area[add + c]->comment,
	      strlen (config.area[add + c]->comment));
      comment[screen.x_max - 40] = 0;
    }

    if ((total[add + c] != last[add + c] + 1) && total[add + c] > 0)
    {
      sprintf (tmpstr, "%-21s %s   %5d NEW %5d",
               name,
               comment,
               total[add + c] - last[add + c] - 1,
               total[add + c]);
      if (c == choice) 
      {
        screen.color = config.cfg.color[MISC_STATUS];
        screen.gotoxy (1, c + 2);
      }
      else
        screen.color = config.cfg.color[EDITOR_QUOTE];
    }
    else
    {
      sprintf (tmpstr, "%-21s %s             %5d",
               name,
               comment,
               total[add + c]);
      if (c == choice) 
      {
        screen.color = config.cfg.color[MISC_STATUS];
        screen.gotoxy (1, c + 2);
      }
      else
        screen.color = config.cfg.color[EDITOR_NORMAL];
    }      

    screen.put (1, 1 + c, tmpstr);

  }
}

/* Select an area */

word READER::choose_area (void)
{
  char done = FALSE;
  char c;
  char tmpstr[256];
  dword n;
  dword searchbox = 0;
  
  total = new dword[config.area_header.areas];
  unread = new dword[config.area_header.areas];
  last = new dword[config.area_header.areas];

  for (n = 0; n < config.area_header.areas; n++)
    msgbase.get_stats (n, &total[n], &last[n], &unread[n]);

  screen.clear (config.cfg.color[EDITOR_NORMAL] << 8);

  screen.put (1, 1, "Area name             Area description                             New     Total", config.cfg.color[MISC_HIGHLIGHT]);
  
  refresh_screen (choice, add);

  memset (tmpstr, 0, 256);
  
  do
  {
    c = keyboard.get ();
    switch (c)
    {
      case 0: /* Extended key */
	c = keyboard.get ();
	switch (c)
	{
	  case 71: /* Home */
            searchbox = 0;
	    choice = 1;
	    add = 0;
	    refresh_screen (choice, add);
            screen.hide_cursor ();
	    break;
	  case 72: /* Up Arrow */
            searchbox = 0;
	    refresh_screen (choice, add);
            screen.hide_cursor ();
	    if (choice > 1) 
	    {
	      choice--;
	      refresh_line (choice, choice, add);
	      refresh_line (choice, choice + 1, add);
	      screen.refresh ();
	    }
	    else
	    {
	      if (add)
	      {
		add--;
		refresh_screen (choice, add);
	      }
	    }
	    break;
	  case 73: /* Page Up */
            searchbox = 0;
	    refresh_screen (choice, add);
            screen.hide_cursor ();
	    if (add >= screen.y_max - 1)
	      add -= screen.y_max - 1;
            else
            {
              add = 0;
              choice = 1;
            }
	    refresh_screen (choice, add);
	    break;
	  case 79: /* End */
            searchbox = 0;
	    refresh_screen (choice, add);
            screen.hide_cursor ();
	    if (config.area_header.areas > screen.y_max - 3) /* more than one page of areas */
	    {
	      choice = screen.y_max - 1;
	      add = config.area_header.areas - choice - 1;
	    }
	    else
	      choice = config.area_header.areas - 1;
	    refresh_screen (choice, add);
	    break;
	  case 80: /* Down Arrow */
            searchbox = 0;
	    refresh_screen (choice, add);
            screen.hide_cursor ();
	    if (choice + add < config.area_header.areas - 1)
	    {
	      if (choice < screen.y_max - 1) 
	      {
		choice++;
		refresh_line (choice, choice, add);
		refresh_line (choice, choice - 1, add);
		screen.refresh ();
	      }
	      else
	      {
		add++;
		refresh_screen (choice, add);
	      }
	    }
	    break;
	  case 81: /* Page Down */
            searchbox = 0;
            screen.hide_cursor ();
	    refresh_screen (choice, add);
	    if (add + (screen.y_max - 1) * 2 < config.area_header.areas)
	      add += screen.y_max - 1;
            else
            {
              add = config.area_header.areas - screen.y_max;
              choice = screen.y_max - 1;
            }
	    refresh_screen (choice, add);
	    break;
	}
	break;
      case 8: /* Backspace */
	if (tmpstr[0])
	  tmpstr[strlen (tmpstr) - 1] = 0;
        screen.put (screen.x_max / 2 - 24 + strlen (tmpstr), screen.y_max / 2,
                    ' ', config.cfg.color[MISC_NORMAL]);
        screen.gotoxy (screen.x_max / 2 - 24 + strlen (tmpstr), screen.y_max / 2);
        screen.show_cursor ();
	screen.refresh ();
	break;
      case 13: /* Return */
        screen.hide_cursor ();
	return (choice + add);
	break;
      case 27: /* Escape */
        if (searchbox == 1)
        {
          searchbox = 0;
          refresh_screen (choice, add);
          screen.hide_cursor ();
        }
        else
        {
          screen.hide_cursor ();
          return 0xFFFF; /* -1 */
        }
	break;
      default: /* Ordinary character */
        if (searchbox == 1)
        {
          tmpstr[strlen (tmpstr)] = toupper (c);
          for (n = 0; n < config.area_header.areas; n++)
            if (strstr (config.area[n]->name, tmpstr))
            {
              if (n > screen.y_max - 1)
                choice = (screen.y_max - 1) / 2;
              else
                choice = n;
              add = n - choice;
              if (add + screen.y_max > config.area_header.areas)
              {
                add = config.area_header.areas - screen.y_max;
                choice = n - add;
              }
              refresh_screen (choice, add);
              break;
            }
        }
        else
        {
          searchbox = 1;
          memset (tmpstr, 0, 256);
          tmpstr[strlen (tmpstr)] = toupper (c);
        }

        screen.window (screen.x_max / 2 - 25, screen.y_max / 2 - 1,
                       screen.x_max / 2 + 25, screen.y_max / 2 + 1, 
                       config.cfg.color[MISC_STATUS],
                       config.cfg.color[MISC_STATUS],
                       "Quick find");
        screen.put (screen.x_max / 2 - 24, screen.y_max / 2,
                    tmpstr, config.cfg.color[MISC_NORMAL]);
        screen.gotoxy (screen.x_max / 2 - 24 + strlen (tmpstr), screen.y_max / 2);
        screen.show_cursor ();
	screen.refresh ();
	break;
    }
  } while (!done);
  screen.clear ();

  delete unread;
  delete total;
  delete last;
}

/* The highest level loop */

void READER::play (void)
{
  word c;

  msgbase.init (config.cfg.msgbase_path);
  screen.save ();

  do
  {
    c = choose_area ();
    if (c != 0xFFFF)
      open_area (c);
  } while (c != 0xFFFF);
  screen.restore ();
}

/* Write a message 
   'type' specifies which type of message we are writing:

   0 = original message (ORIGINAL)
   1 = reply (REPLY)
   2 = reply to recipient (REPLY_TO_RECIPIENT)
   3 = forward (FORWARD) */

void READER::write_message (msghdr_type msghdr, byte *oldmsg, word area, word type)
{
  char *msgtxt = NULL, *txt = NULL;
  char *buf;
  char tmpstr[256], *tmpstr2;
  char quote[10];
  dword c, d;
  dword e;
  char reply[80], *tmp;
  tm *tblock;
  FILE *file;

  if (area == 0xFFFF)
    return;

  screen.clear ();

  switch (type)
  {
    case ORIGINAL: /* New message */
      if (area == 1) /* netmail */
      {
	txt = NULL;
      }
      else
      {
	txt = new char[256];
	memset (txt, 0, 256);
      }
      memset (&msghdr, 0, sizeof (msghdr_type));
      msghdr.folder = area;
      memset (msghdr.sender, 0, 36);
      strcpy ((char *) msghdr.sender, config.cfg.user_name);
      msghdr.status_flags = 1 | FLAG_SEEN; /* local and seen */
      msghdr.time_written = time (NULL);
      tblock = localtime ((time_t *) &msghdr.time_written);
      msghdr.time_written += tblock->tm_gmtoff;
      msghdr.sender_addr = config.cfg.aka[config.area[msghdr.folder]->aka];
      msghdr.reply_child = msghdr.reply_mother = msghdr.reply_next = msghdr.reply_prev = MAX;
      break;
    case REPLY: /* Reply */
    case REPLY_TO_RECIPIENT: /* Reply to original sender */
      /* Compose a string containing the senders initials */
      
      quote[0] = 0;
      
      if (config.cfg.quote_type == 1 || config.cfg.quote_type == 2)
      {
	quote[0] = msghdr.sender[0];
	for (c = 1, d = 1; msghdr.sender[c]; c++)
	{
	  if (msghdr.sender[c] == ' ' || msghdr.sender[c] == '-')
	    quote[d++] = msghdr.sender[c + 1];
	}
	quote[d] = 0;

	convert_charset (quote, 1, 0);
          
	if (config.cfg.quote_type == 1) /* xy> */
	  to_lower (quote);
	else /* XY> */
	  to_upper (quote);

	convert_charset (quote, 1, 0);
      }      
      strcat ((char *) quote, ">");
      
      if (type == REPLY) /* Regular reply */
      {
	memset (msghdr.recipient, 0, 36);
	strcpy ((char *) msghdr.recipient, (char *) msghdr.sender);
      }

      memset (msghdr.sender, 0, 36);
      strcpy ((char *) msghdr.sender, config.cfg.user_name);
      msghdr.status_flags = 1 | FLAG_SEEN; /* local */
      msghdr.time_written = time (NULL);
      tblock = localtime ((time_t *) &msghdr.time_written);
      msghdr.time_written += tblock->tm_gmtoff;
      msghdr.recipient_addr = msghdr.sender_addr;
      msghdr.sender_addr = config.cfg.aka[config.area[msghdr.folder]->aka];
      msghdr.reply_child = msghdr.reply_mother = msghdr.reply_next = msghdr.reply_prev = MAX;
      
      txt = new char[msglen + msglen]; // FIXME: this *must* be replaced with the real expression!
      memset (txt, 0, msglen + msglen);
      
      /* If this is a cross-reply, put in relevant information */
      
      if (area != msghdr.folder)
      {
	sprintf (txt, " * Reply to a message in %s\r\r", config.area[msghdr.folder]->name);
	msghdr.folder = area;
      }
      
      /* Quote the text */
      
      for (e = 0; e < msglen;)
      {
	if (is_quote ((char *) oldmsg + e) || !*(char *) (oldmsg + e))
	{
	  strcat (txt, (char *) oldmsg + e);
	  strcat (txt, "\r");
	}
	else if (!is_kludge ((char *) oldmsg + e))
	{
	  strcat (txt, " ");
	  strcat (txt, (char *) quote);
	  strcat (txt, " ");
	  strcat (txt, (char *) oldmsg + e);
	  strcat (txt, "\r");
	}
	else /* could be MSGID kludge */
	{
	  tmp = get_string ((char *) oldmsg + e, "MSGID: ");
	  if (tmp[0]) 
	    strcpy (reply, tmp);
	}
	e += strlen ((char *) oldmsg + e) + 1;
      }
      break;
    case FORWARD: /* Forward */
      txt = new char[msghdr.txt_size * 2];
      memset (txt, 0, msghdr.txt_size * 2);
      
      sprintf (tmpstr, "* Forwarded from %s\r", config.area[msghdr.folder]);
      strcat (txt, tmpstr);
      if (msghdr.folder != 1) /* non-netmail */
      {
	sprintf (tmpstr, "* Originally from %s to %s\r", msghdr.sender,
                 msghdr.recipient);
      }
      else
      {
	sprintf (tmpstr, "* Originally from %s (%s) to %s (%s)\r",
		 msghdr.sender, binToStrAddr (msghdr.sender_addr), 
		 msghdr.recipient, binToStrAddr (msghdr.recipient_addr));
      }
      strcat (txt, tmpstr);
      strcat (txt, "\r");
     
      for (c = 0; c < msghdr.txt_size;)
      {
	if (!is_kludge ((char *) oldmsg + c))
	{
	  if (!strncmp ((char *) oldmsg + c, " * Origin:", 10))
	  {
	    strcat (txt, " + Origin:");
	    c += 10;
	  }
	  else
	    if (!strncmp ((char *) oldmsg + c, "---", 3 ))
	    {
	      strcat (txt, "-+-");
	      c += 3;
	    }
	  if (c < msghdr.txt_size)
	    {
	      strcat (txt, (char *) oldmsg + c);
	      strcat (txt, "\r");
	      c += strlen ((char *) oldmsg + c) + 1;
	    }
	}
	else
	  c += strlen ((char *) oldmsg + c) + 1;
      }
      msghdr.folder = area;
      break;
  }

  if (msghdr.folder != 1)                  /* No origin if netmail */
  {
    /* This function is not documented, but if a file named 'origin'
       is put in the .tanstaafl directory, it will be used as an origin. */

    FILE *file = NULL;
    char tmp[256];
    char *origin;

    sprintf (tmp, "%s/.tanstaafl/origin", getenv ("HOME"));

    file = fopen (tmp, "rt");
    if (file != NULL)
    {
      origin = (char *) malloc (256);
      fgets (origin, 256, file);
      fclose (file);
      if (strlen (origin) + strlen ("* Origin:") +
          strlen (binToStrAddr (config.cfg.aka[config.area[msghdr.folder]->aka])) +
          3 > 80)
      {
        origin[80 - (strlen ("* Origin:") +
                     strlen (binToStrAddr (config.cfg.aka[config.area[msghdr.folder]->aka])) +
                     3)] = '\0';
      }

    }
    else
    {
      origin = config.cfg.point_name;
    }

    sprintf (tmpstr, "\r\r * Origin: %s (%s)\r", origin,
	     binToStrAddr (config.cfg.aka[config.area[msghdr.folder]->aka]));
    strcat (txt, tmpstr);
  }
  
  if (editor.edit_header (&msghdr) != ABORT)
  {
    msgtxt = editor.edit_message (txt, &msghdr, 1);
    delete txt;
  
    if (msgtxt != NULL)
    {
      buf = new char[strlen (msgtxt) + 1024];
      memset (buf, 0, strlen (msgtxt) + 1024);

      /* Add appropriate kludges to the text */

      if (msghdr.sender_addr.point && msghdr.folder == 1)
      {
	sprintf (tmpstr, "\001FMPT %d\r", msghdr.sender_addr.point);
	strcat (buf, tmpstr);
      }

      if (msghdr.recipient_addr.point && msghdr.folder == 1)
      {
	sprintf (tmpstr, "\001TOPT %d\r", msghdr.recipient_addr.point);
	strcat (buf, tmpstr);
      }
	  
      if (msghdr.recipient_addr.zone != msghdr.sender_addr.zone && msghdr.folder == 1)
      {
	sprintf (tmpstr, "\001INTL %s %s\r", binToStrAddr (msghdr.recipient_addr), binToStrAddr (msghdr.sender_addr));
	strcat (buf, tmpstr);
      }

      sprintf (tmpstr, "\001PID: %s %s (build %d)\r", program_name, program_version, build_number);
      strcat (buf, tmpstr);
	 
      strcat (buf, "\001CHRS: IBMPC 2\r"); // FIXME: we should support other charsets too..
	  
      sprintf (tmpstr, "\001MSGID: %s %x\r", binToStrAddr (msghdr.sender_addr), time (NULL));
      strcat (buf, tmpstr);

      if ((type == REPLY || type == REPLY_TO_RECIPIENT) && msghdr.msgid_CRC)
      {
	sprintf (tmpstr, "\001REPLY: %s\r", reply);
	strcat (buf, tmpstr);
	tmpstr2 = get_string (buf, "\001REPLY: ");
	msghdr.reply_CRC = crc32buf (tmpstr2, strlen (tmpstr2));
      }              
      else
	msghdr.reply_CRC = 0;

      strcat (buf, msgtxt);

      tmpstr2 = get_string (buf, "\001MSGID: ");
      msghdr.msgid_CRC = crc32buf (tmpstr2, strlen (tmpstr2));
	  
	  
      msghdr.txt_size = strlen (buf);
      msgbase.close_area ();
      e = msgbase.write_message (msghdr, buf);
      msgbase.open_area (last_area, &curmsg);

      delete buf;
      buf = NULL;

      /* Append the message number to the messages.tfl file in the system directory */

      sprintf (tmpstr, "%s/messages.tfl", config.cfg.system_path);
      file = fopen (tmpstr, "ab");
      fwrite (&e, 4, 1, file);
      fclose (file);
          
      msgbase.link (msgbase.get_number_of_messages () - 1, msgbase.get_number_of_messages ());
    }
  }
}

/* Export a message to a text file */

void READER::export_message (dword message, word area_number, byte flags)
{
  Window *win;
  char filename[80] = "";
  char estr[256], tmp[256], *t2;
  int i;
  struct tm *tbclock;
  FILE *file;

  getcwd (tmp, 256);
  if (getenv ("HOME"))
    chdir (getenv ("HOME"));

  win = new Window (1, screen.y_max / 2, screen.x_max, 3, config.cfg.color[MISC_WINDOW], config.cfg.color[MISC_TITLE], 
		    "Enter filename");
  screen.refresh ();
  win->get_line (2, 1, filename, 79, TRUE);  
  delete win;
  if (!filename[0]) return;

  if (!(file = fopen(filename, "w"))) 
  {
    sprintf (estr, "The file \"%s\" couldn't be opened for writing", 
	     filename);
    ui.infobox ("Error", estr);
    return;
  }

  /* Write message number and area */

  t2 = strdup ((char *) config.area[area_number]->name);
#ifdef __linux__
  convert_charset ((uchar *) t2, 1, 0);
#endif
  for (i = 0; i < 80; i++)
    fputc('-', file);
  fprintf (file, "\nMessage number %u from area %s\n", message, 
	   t2);
  for (i = 0; i < 80; i++)
    fputc('-', file);

  /* Write the header */

  tbclock = gmtime ((const time_t *) &msghdr->time_written);
  fprintf (file, "\nDate : %s %s %02d, %02d:%02d\n", month[tbclock->tm_mon],
	   weekday[tbclock->tm_wday], tbclock->tm_mday, tbclock->tm_hour,
	   tbclock->tm_min);
  t2 = strdup ((char *) msghdr->sender);
#ifdef __linux__
  convert_charset ((uchar *) t2, 1, 0);
#endif
  fprintf (file, "From : %s (%u:%u/%u.%u)\n", t2, 
	   msghdr->sender_addr.zone, msghdr->sender_addr.net, 
	   msghdr->sender_addr.node, msghdr->sender_addr.point);

  t2 = strdup ((char *) msghdr->recipient);
#ifdef __linux__
  convert_charset ((uchar *) t2, 1, 0);
#endif
  if (area_number == 1)
    fprintf (file, "To   : %s (%u:%u/%u.%u)\n", t2,
	     msghdr->recipient_addr.zone, msghdr->recipient_addr.net,
	     msghdr->recipient_addr.node, msghdr->recipient_addr.point);
  else
    fprintf (file, "To   : %s\n", t2);

  t2 = strdup ((char *) msghdr->subject);
#ifdef __linux__
  convert_charset ((uchar *) t2, 1, 0);
#endif
  fprintf (file, "Subj : %s\n", t2);
  fprintf (file, "Flag : ");
  if (msghdr->status_flags & FLAG_PRIVATE) fprintf(file, "pvt ");
  if ((msghdr->status_flags & FLAG_LOCAL) && (msghdr->status_flags & FLAG_SENT))
    fprintf(file, "snt ");
  if ((area_number == 1) && !(msghdr->status_flags & 1) && 
      (msghdr->status_flags & FLAG_READ)) fprintf(file, "rcv ");
  if (msghdr->status_flags & FLAG_LOCAL) fprintf(file, "loc");
  fputc ('\n', file);
  for (i = 0; i < 80; i++) fputc ('-', file);
 
  /* Write the message body */


  i = 0;
  while (i < msghdr->txt_size)
  {
    if (!(flags & 1)) 
    {
      while (is_kludge ((char *) msgbase.msgtxt + i))
	i += strlen ((char *) msgbase.msgtxt + i) + 1;
    }
#ifdef __linux__
    convert_charset ((uchar *) msgbase.msgtxt + i, 1, 0);
#endif
    if (is_kludge ((char *) msgbase.msgtxt + i)) 
    {
      putc ('@', file);
      i++;
    }
    fprintf (file, "%s", msgbase.msgtxt + i);
    i += strlen ((char *) msgbase.msgtxt + i);
    putc ('\n', file);
    i++;
  }

  fclose (file);
  chdir (tmp);
}

/* Help screen */

void READER::help (void)
{
  Window *win;
  byte c;

  win = new Window (0, 0, 65, 17, config.cfg.color[MISC_WINDOW], config.cfg.color[MISC_TITLE], "Help");
  win->color = config.cfg.color[MISC_NORMAL];
  win->put ("E            Export message to file");
  win->put ("G            Goto a message number");
  win->put ("L            Enter list-mode");
  win->put ("N            Reply in other area (cross-reply)");
  win->put ("O            ROT13-decode the current message");
  win->put ("R            Reply");
  win->put ("Ins          Enter a new message");
  win->put ("Left Arrow   Go to the previous message");
  win->put ("Right Arrow  Go to the following message");
  win->put ("Esc          Return to area selector");
  win->put ("Ctrl-F       Forward message to other area");
  win->put ("Ctrl-R       Reply to recipient");
  win->put ("Alt-H        Edit header (only on locally created)");
  win->put ("Alt-S        Search for text in area");
  win->put ("Alt-N        Reply in netmail");
  screen.refresh ();

  do
  {
    c = keyboard.get ();
  } while (c != 13 && c != 27);
  delete win;
}

/* Search in the current area */

void READER::search (word area_number, byte flags)
{
  Window *win;
  LISTBOX *lbx;
  static byte sstr[128] = "";
  search_t srch;
  dword i;
  int c;
  int s;

  win = new Window (1, screen.y_max / 2, screen.x_max, 3, 
		    config.cfg.color[MISC_WINDOW], 
		    config.cfg.color[MISC_TITLE], "Enter searchstring");
  win->put (2, 1, (char *) sstr, config.cfg.color[MISC_NORMAL]);
  screen.refresh();
  win->get_line (2, 1, (char *) sstr, 127, TRUE);
  delete win;
  
  if (!sstr[0]) return;

  lbx = new LISTBOX (10, 3, screen.x_max / 2, 0, "Search order");
  lbx->insert ("Forward from current message", (char *) NULL);
  lbx->insert ("Backward from current message", (char *) NULL);
  lbx->insert ("From beginning of area", (char *) NULL);
  lbx->insert ("From end of area", (char *) NULL);

  c = lbx->play (1);
  delete lbx;
  screen.refresh ();

  keyboard.done ();

  win = new Window (10, 10, 35, 3, config.cfg.color[MISC_WINDOW],
		    config.cfg.color[MISC_TITLE], "Searching...");
  win->put (2, 1, "Hit the any key to abort search", 
	    config.cfg.color[MISC_NORMAL]);
  screen.refresh ();
  
  if (!(srch = tns_search_init ((uchar *)sstr))) return;
  
  switch (c) 
    {
    case ABORT:
      return;
      break;
    case 1:
      for (i = curmsg + 1; i < total[area_number]; i++) 
	if ((s = search_in_msg (srch, i))) break;
      break;
    case 2:
      if (!curmsg)
	return;
      for (i = curmsg - 1; i >= 0; i--)
	if ((s = search_in_msg (srch, i))) break;
      break;
    case 3:
      for (i = 0; i < (int)total[area_number]; i++)
	if ((s = search_in_msg (srch, i))) break;
      break;
    case 4:
      if (!total[area_number])
	return;
      for (i = total[area_number] - 1; i >= 0; i--)
	if ((s = search_in_msg (srch, i))) break;
      break;
    }
  delete win;

  tns_search_destroy (srch);

  if (s < 1)
    msgbase.read_message (curmsg, msghdr);
  else if (s == 1)
    show_message (curmsg, area_number, 0, flags);

  keyboard.init();
}

/* Search for a text in the current message */

int READER::search_in_msg (__search_t *srch, dword message)
{
  //  if (keyboard.hit ()) return -1;
 
  if (msgbase.read_message (message, msghdr) == ABORT) 
    {
      msgbase.read_message (curmsg, msghdr);
      return 0;
    }
  if (tns_search (srch, msgbase.msgtxt, msghdr->txt_size))
    {
      if (curmsg == message) return 2;
      curmsg = message;
      return 1;
    }
  return 0;
}
