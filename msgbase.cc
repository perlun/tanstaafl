/* msgbase.cc - Routines for reading and writing to a tanstaafl message base */
/* Copyright 1998-1999 Per Lundberg */

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

#include <fstream>
#include <stdio.h>

MSGBASE::MSGBASE (string msgbase_path)
{
  init (msgbase_path);
  MSGBASE ();
}

MSGBASE::MSGBASE (void)
{
  msg = NULL;
  msgtxt = NULL;
}

MSGBASE::~MSGBASE (void)
{
}

bool MSGBASE::init (string msgbase_path)
{
  MSGBASE::msgbase_path = msgbase_path;
}

/* Open an area */

char MSGBASE::open_area (word area_number, dword *last_message)
{
  /* Make a list of the messages who belong to this area */

  word *buffer;
  dword len, c;
  FILE *file;
  char tmpstr[256];
  
  area = area_number;
  messages = 0;
  MSGBASE::last_message = MAX; /* -1 */

  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msgidx.tfl");

  if ((file = fopen (tmpstr, "rb")) == NULL) /* the fact that the file does not exist is probably because we haven't tossed any mail yet */
  {
    return SUCCESS;
  }
  
  if (msg != NULL)
  {
    delete msg;
    msg = NULL;
  }

  fseek (file, 0, SEEK_END);
  len = ftell (file);
  buffer = new word[len / 2];
  msg = new dword[len / 2 + 1];
  
  memset (msg, 0, len);
  fseek (file, 0, SEEK_SET);
  fread (buffer, len, 1, file);
  
  for (c = 0; c < len / 2; c++)
  {
    if (buffer[c] == area)
    {
      msg[messages++] = c;
    }
  }
  
  delete buffer;
  buffer = NULL;

  fclose (file);	

  /* Get the lastread message */

  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msgstat.tfl");
  file = fopen (tmpstr, "rb");
  if (file == NULL)
  {
    ui.infobox ("Error", string ("Could not open ") + string (tmpstr));
    return ABORT;
  }
  fseek (file, 256 + 8000 + area * 4, SEEK_SET);
  fread (last_message, 4, 1, file);
  fclose (file);
  if (*last_message > messages) 
  {
    *last_message = messages;
  }

  return TRUE;
}

/* Open a message area, but do not make it the current (with regards to
   the lastread pointer and stuff like that */

void MSGBASE::open (dword area_number)
{
  /* Make a list of the messages who belong to this area */

  word *buffer;
  dword len, c;
  FILE *file;
  char tmpstr[256];
  
  messages = 0;
  MSGBASE::last_message = MAX; /* -1 */

  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msgidx.tfl");

  if ((file = fopen (tmpstr, "rb")) == NULL) /* the fact that the file does not exist is probably because we haven't tossed any mail yet */
    {
      return;
    }

  if (msg != NULL)
    {
      delete msg;
      msg = NULL;
    }

  fseek (file, 0, SEEK_END);
  len = ftell (file);
  buffer = new word[len / 2];
  msg = new dword[len / 2 + 1];
  
  memset (msg, 0, len);
  fseek (file, 0, SEEK_SET);
  fread (buffer, len, 1, file);
  
  for (c = 0; c < len / 2; c++)
    if (buffer[c] == area_number)
      msg[messages++] = c;
  
  delete buffer;
  buffer = NULL;

  fclose (file);	
}

/* Get number of unseen messages in area */

dword MSGBASE::get_new (dword area_number)
{
#if 0
  dword len, c, msgs = 0;
  FILE *file;
  char tmpstr[256];
  
  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msgidx.tfl");

  if ((file = fopen (tmpstr, "rb")) == NULL) /* the fact that the file does not exist is probably because we haven't tossed any mail yet */
  {
    return 0;
  }
  
  fseek (file, 0, SEEK_END);
  len = ftell (file);
  buffer = new word[len / 2];
  
  memset (msg, 0, len);
  fseek (file, 0, SEEK_SET);
  fread (buffer, len, 1, file);
  
  for (c = 0; c < len / 2; c++)
    if (buffer[c] == area_number)
      msgs++;
  
  delete buffer;
  buffer = NULL;

  fclose (file);	

  return msgs;
#endif
  return 666;
}

/* Close the currently open area */

bool MSGBASE::close_area (void)
{
  char tmpstr[256];
  FILE *file;

  if (msg != NULL)
    {
      delete msg;
      msg = NULL;
    }

  /* Update the lastread pointer */
   
  if (last_message != MAX)
    {
      strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
      tmpstr[msgbase_path.length ()] = 0;
      strcat (tmpstr, "/msgstat.tfl");
      file = fopen (tmpstr, "rb+"); /* open for read and write */
      if (file == NULL)
	{
	  ui.infobox ("Error", string ("Could not open ") + string (tmpstr));
	  return ABORT;
	}
      fseek (file, 256 + 8000 + area * 4, SEEK_SET);
      fwrite (&last_message, 4, 1, file);
      fclose (file);
    }
  return SUCCESS;
}

/* Read a message from the current message area (note that 'message' here
   specifies the message number _within the area_; not the "real" message
   number */

char MSGBASE::read_message (dword message, msghdr_type *msghdr)
{
  if (message > messages) return ABORT;

  if (read_message_header (msg[message], msghdr) == ABORT)
    {
      ui.infobox ("Error", "Could not read message header");
      return ABORT;
    }

  if (read_message_text (msg[message], *msghdr) == ABORT)
    {
      ui.infobox ("Error", "Could not read message text");
      return ABORT;
    }

  last_message = message;

  return SUCCESS;
}

/* Read the message header of the specified (linear) message */

char MSGBASE::read_message_header (dword message, msghdr_type *msghdr)
{
  char tmpstr[256];
  FILE *file;
  
  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msghdr.tfl");

  if ((file = fopen (tmpstr, "rb")) == NULL)
  {
    ui.infobox ("Error", string ("Could not open ") + string (tmpstr));
    return ABORT;
  }

  fseek (file, sizeof (msghdr_type) * message, SEEK_SET);
  fread (msghdr, sizeof (msghdr_type), 1, file);
  fclose (file);

  return SUCCESS;
}

/* Read the message text of the specified (linear) message */

char MSGBASE::read_message_text (dword message, msghdr_type msghdr)
{
  FILE *file;
  char tmpstr[256];

  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msgtxt.tfl");
  
  if ((file = fopen (tmpstr, "rb")) == NULL)
  {
    ui.infobox ("Error", string ("Could not open ") + string (tmpstr));
    return ABORT;
  }

  fseek (file, msghdr.txt_start, SEEK_SET);

  if (msgtxt != NULL)
    {
      delete msgtxt;
      msgtxt = NULL;
    }

  msgtxt = new byte[msghdr.txt_size + 1];
  fread (msgtxt, msghdr.txt_size, 1, file);
  msgtxt[msghdr.txt_size] = 0;
  fclose (file);

  return SUCCESS;
}

/* Write a message to the message base */
// FIXME: Write this

bool MSGBASE::write_message (dword message, msghdr_type msghdr, char *text)
{
  return ABORT;
}

/* Add a message to the message base */
/* Returns the linear message number if succeeded, MAX if error */ 

dword MSGBASE::write_message (msghdr_type msghdr, char *text)
{
  FILE *file;
  char tmpstr[256];
  msgstat_type msgstat;
  dword n;

  /* Add the message text to msgtxt.tfl */

  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msgtxt.tfl");  

  file = fopen (tmpstr, "ab");
  if (file == NULL)
    {
      ui.infobox ("Error", string ("Could not open ") + string (tmpstr) +
		  string (" for writing"));
      return MAX;
    }
  msghdr.txt_start = ftell (file);
  fwrite (text, strlen (text), 1, file);
  fclose (file);
  
  /* Add the message header to msghdr.tfl */

  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msghdr.tfl");  

  file = fopen (tmpstr, "ab");
  if (file == NULL)
    {
      ui.infobox ("Error", string ("Could not open ") + string (tmpstr) +
		  string (" for writing"));
      return MAX;
    }
  fwrite (&msghdr, sizeof (msghdr_type), 1, file);
  fclose (file);
  
  /* Add a record to msgidx.tfl */

  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msgidx.tfl");  

  file = fopen (tmpstr, "ab");
  if (file == NULL)
    {
      ui.infobox ("Error", string ("Could not open ") + string (tmpstr));
      return MAX;
    }
  n = ftell (file) / 2;
  fwrite (&msghdr.folder, 2, 1, file);
  fclose (file);

  /* Update msgstat.tfl */

  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msgstat.tfl");  

  file = fopen (tmpstr, "rb");
  if (file == NULL) /* file does not exist */
    memset (&msgstat, 0, sizeof (msgstat_type));
  else
    fread (&msgstat, sizeof (msgstat_type), 1, file);

  msgstat.messages[msghdr.folder]++;
  msgstat.unread[msghdr.folder]++;

  if (file != NULL)
    fclose (file);

  file = fopen (tmpstr, "wb");
  if (file == NULL)
    {
      ui.infobox ("Error", string ("Could not open ") + string (tmpstr));
      return MAX;
    }
  fwrite (&msgstat, sizeof (msgstat_type), 1, file);
  fclose (file);

  messages++;

  return n;
}
 
/* Write a message header to the message base */

bool MSGBASE::write_message_header (dword message, msghdr_type msghdr)
{
  char tmpstr[256];
  FILE *file;

  // if (message > messages) return ABORT;
  
  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msghdr.tfl");

  if ((file = fopen (tmpstr, "rb+")) == NULL)
  {
    ui.infobox ("Error", string ("Could not open ") + string (tmpstr));
    return ABORT;
  }

  fseek (file, sizeof (msghdr_type) * message, SEEK_SET);
  fwrite (&msghdr, sizeof (msghdr_type), 1, file);
  fclose (file);

  return SUCCESS;
}

/* Delete a message from the message base */

bool MSGBASE::delete_message (dword message)
{
  FILE *file;
  char tmpstr[256];
  dword c;
  msghdr_type msghdr;
  msgstat_type msgstat;

  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msgidx.tfl");
  file = fopen (tmpstr, "rb+");

  if (file == NULL)
    {
      ui.infobox ("Error", string ("Could not open ") + string (tmpstr));
      return ABORT;
    }

  fseek (file, message * 4, SEEK_SET);
  c = 65535;                                 /* Messages with the folder number 65535 is treated as "deleted" */
  fwrite (&c, 4, 1, file);
  fclose (file);

  /* Flag the message as unread in msghdr.tfl */

  read_message_header (message, &msghdr);
  msghdr.status_flags |= FLAG_DELETED;         /* deleted */
  write_message_header (message, msghdr);

  /* Modify the data in msgstat.tfl */

  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msgstat.tfl");
  file = fopen (tmpstr, "rb+");
  
  if (file == NULL)
    {
      ui.infobox ("Error", string ("Could not open ") + string (tmpstr));
      return ABORT;
    }

  fread (&msgstat, sizeof (msgstat_type), 1, file);
  msgstat.messages[messages]--;
  fseek (file, 0, SEEK_SET);
  fwrite (&msgstat, sizeof (msgstat_type), 1, file);
  fclose (file);

  return SUCCESS;
}

/* Get message statistics for the chosen area */

bool MSGBASE::get_stats (word area, dword *messages, dword *last, dword *unread)
{
  FILE *file;
  char tmpstr[256];

  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msgstat.tfl");
  file = fopen (tmpstr, "rb");
  if (file == NULL) /* the fact that msgstat.tfl doesn't exist isn't an error; it is generated when messages are written to the message base */
    {
      *messages = 0;
      *last = 0; 
      *unread = 0;
      return ABORT;
    }
  fseek (file, 256 + area * 4, SEEK_SET);
  fread (messages, 4, 1, file);
  fseek (file, 256 + 8000 + area * 4, SEEK_SET);
  fread (last, 4, 1, file);
  fseek (file, 256 + 16000 + area * 4, SEEK_SET);
  fread (unread, 4, 1, file);
  fclose (file);

  return SUCCESS;
}

/* Rebuild msgstat.tfl from msghdr.tfl */

bool MSGBASE::rebuild_msgstat (void)
{
  FILE *file;
  char tmpstr[256];
  msgstat_type msgstat;
  msghdr_type msghdr;
  word c;

  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msgstat.tfl");
  file = fopen (tmpstr, "rb");
 
  if (file == NULL)
    {
      memset (&msgstat, 0, sizeof (msgstat_type));
      msgstat.msgbase_version = 1;
    }
  else
    {
      fread (&msgstat, sizeof (msgstat_type), 1, file);
      memset (&msgstat.messages[0], 0, 2000 * 4);
      memset (&msgstat.unread[0], 0, 2000 * 4);
      fclose (file);
    }
  
  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msgidx.tfl");
  file = fopen (tmpstr, "rb");
 
  if (file == NULL)
    {
      ui.infobox ("Error", string ("Could not open ") + string (tmpstr));
      return ABORT;
    }

  while (!feof (file))
    {
      //fread (&msghdr, sizeof (msghdr_type), 1, file);
      //if ((msghdr.status_flags & 9) == 0) /* not local, not read */
      // 	msgstat.unread[msghdr.folder]++;
      //      if ((msghdr.status_flags & 0x80000000) == 0) /* not deleted */
      fread (&c, 2, 1, file);
      msgstat.messages[c]++;
    }

  /* Write the data to msgstat.tfl */

  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msgstat.tfl");
  file = fopen (tmpstr, "wb");

  if (file == NULL)
    {
      ui.infobox ("Error", string ("Could not open ") + string (tmpstr));
      return ABORT;
    }

  fwrite (&msgstat, sizeof (msgstat_type), 1, file);
  fclose (file);

  return SUCCESS;
}

/* Get the total number of messages in the message base */

dword MSGBASE::get_number_of_messages (void)
{
  FILE *file;
  char tmpstr[256];
  dword messages;

  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msghdr.tfl");
  file = fopen (tmpstr, "rb");
  
  if (file == NULL)
    {
      ui.infobox ("Error", string ("Could not open ") + string (tmpstr));
      return MAX;
    }

  fseek (file, 0, SEEK_END);
  messages = ftell (file) / sizeof (msghdr_type);
  fclose (file);
  return messages;
}

/* Perform reply linking of the message base, starting at message number 
   'start' */

void MSGBASE::link (dword start, dword end)
{
  char tmpstr[256];
  FILE *file;
  dword messages;
  dword *msgid_CRC, *reply_CRC;
  msghdr_type msghdr;
  dword d, e;
  byte mode = 0;

  strncpy (tmpstr, msgbase_path.data (), msgbase_path.length ());
  tmpstr[msgbase_path.length ()] = 0;
  strcat (tmpstr, "/msghdr.tfl");
  file = fopen (tmpstr, "rb+");

  if (file == NULL)
    {
      ui.infobox ("Error", string ("Could not open ") + string (tmpstr));
      return;
    }

  fseek (file, 0, SEEK_END);
  messages = ftell (file) / sizeof (msghdr_type);

  if (end > messages) 
    end = messages;

  msgid_CRC = new dword[messages];
  reply_CRC = new dword[messages];

  if (end - start < 500) /* If linking less than 500 messages, don't
                            buffer the CRC values, since that's very time
                            consuming on large message bases */
    {
      memset (msgid_CRC, 0xFF, messages * 4);
      memset (reply_CRC, 0xFF, messages * 4);
      mode = 1;
    }
  else
    {
      /* Lets start by building a list of all msgid_CRC and reply_CRC values */

      fseek (file, 0, SEEK_SET);
      
      for (dword c = 0; c < messages; c++)
        {
          if (fread (&msghdr, sizeof (msghdr_type), 1, file) == -1)
            {
              ui.infobox ("Error", "Could not read message header");
              return;
            }
          msgid_CRC[c] = msghdr.msgid_CRC;
          reply_CRC[c] = msghdr.reply_CRC;
        }
    }

  keyboard.done ();

  /* Now that we've got a full list of all MSGID and REPLY CRCs, we can
     do the actual linking on the chosen range of messages */

  for (dword c = start; c < end; c++)
    {
      d = c;
      do
	{
          if (mode == 1) /* the CRC:s are not buffered, so we'll have to read
                            them from the file */
            {
              if (reply_CRC[c] == MAX)
                {
                  fseek (file, sizeof (msghdr_type) * c, SEEK_SET);
                  fread (&msghdr, sizeof (msghdr_type), 1, file);
                  reply_CRC[c] = msghdr.reply_CRC;
                }
              
              if (msgid_CRC[d] == MAX)
                {
                  fseek (file, sizeof (msghdr_type) * d, SEEK_SET);
                  fread (&msghdr, sizeof (msghdr_type), 1, file);
                  msgid_CRC[d] = msghdr.msgid_CRC;
                }
            }
#ifdef DEBUG
          printf ("%d %d 0x%08X 0x%08X\n", c, d, reply_CRC[c], msgid_CRC[d]);
#endif

          if (!msgid_CRC[d]) /* message doesn't contain a message ID string */
            {
              d--;
              continue;
            }

	  if (!reply_CRC[c]) /* message doesn't contain a reply ID string */
            {
              d = 0;
              continue;
            }

	  if (reply_CRC[c] == msgid_CRC[d]) /* message c is a reply to message d */
	    {
#ifdef DEBUG
	      printf ("%d %d\n", c, d);
#endif
	      fseek (file, sizeof (msghdr_type) * d, SEEK_SET);
	      
	      if (fread (&msghdr, sizeof (msghdr_type), 1, file) == -1)
		{
		  ui.infobox ("Error", "Could not read message header");
		  return;
		}
	      
	      if (msghdr.reply_child != MAX) /* Message c is not the only reply of message d */
		{
		  /* Find the last message that is a reply to this message and 
		     update it's reply_next pointer */
		  
		  e = msghdr.reply_child;
		  do
		    {
		      fseek (file, sizeof (msghdr_type) * e, SEEK_SET);
		      fread (&msghdr, sizeof (msghdr_type), 1, file);
		      if (e == msghdr.reply_next) /* message points to itself */
			break;
		      if (msghdr.reply_next != MAX)
			e = msghdr.reply_next;
		    } while (msghdr.reply_next != MAX);
		  
		  msghdr.reply_next = c;
		  fseek (file, sizeof (msghdr_type) * e, SEEK_SET);
		  fwrite (&msghdr, sizeof (msghdr_type), 1, file);
		  
		  /* Update the mother and prev fields of message c */

		  fseek (file, sizeof (msghdr_type) * c, SEEK_SET);
		  fread (&msghdr, sizeof (msghdr_type), 1, file);
		  msghdr.reply_mother = d;
		  msghdr.reply_prev = e;
		  fseek (file, sizeof (msghdr_type) * c, SEEK_SET);
		  fwrite (&msghdr, sizeof (msghdr_type), 1, file);
		}
	      else /* This is the first reply to this message */
		{
		  msghdr.reply_child = c;
		  fseek (file, sizeof (msghdr_type) * d, SEEK_SET);
		  fwrite (&msghdr, sizeof (msghdr_type), 1, file);
		  
		  fseek (file, sizeof (msghdr_type) * c, SEEK_SET);
		  fread (&msghdr, sizeof (msghdr_type), 1, file);
		  msghdr.reply_mother = d;
		  fseek (file, sizeof (msghdr_type) * c, SEEK_SET);
		  fwrite (&msghdr, sizeof (msghdr_type), 1, file);
		}
	    }
          d--;
	} while (d && reply_CRC[c] != msgid_CRC[d + 1]);
    }
  
  fclose (file);

  delete msgid_CRC;
  delete reply_CRC;
  keyboard.init ();
}

/* Convert a linear message number to a logic (in the current area) number */

dword MSGBASE::linear_to_logic (dword message)
{
  dword c;

  for (c = 0; c < messages; c++)
    if (msg[c] == message) return c;

  return MAX;
}

/* Get the area number for a linear message number */

dword MSGBASE::area_number (dword message) 
{
  msghdr_type msghdr;

  if (read_message_header (message, &msghdr) == ABORT)
    return MAX;
  return msghdr.folder;
}
