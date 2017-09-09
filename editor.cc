/* editor.cc - the tanstaafl editor */
/* Copyright 1998-1999 Per Lundberg */
/* Copyright 1999 Bartek Kania */

#include "tanstaafl.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

EDITOR::EDITOR (void)
{
  msgbuf = NULL;
}

EDITOR::~EDITOR (void)
{
}

/* Edit a string on the screen */

byte EDITOR::read_string (uchar x, uchar y, char *tmp_str, uchar max_len)
{
  char *old_str = new char[256];
  char *tmp_str2;
  uchar c, l = strlen (tmp_str), m = l, help = 0;
  bool insert = TRUE;

  memset (old_str, 0, 256);
  strcpy (old_str, tmp_str);
  screen.gotoxy (x + l, y);
  
  screen.show_cursor ();  
  do
  {
    c = keyboard.get ();
    switch (c)
    {
      case 0:
	c = keyboard.get ();
	switch (c)
	{
	  case 33: /* Alt-F */
	    screen.hide_cursor ();
	    return 'f';
	    break;

	  case 35: /* Alt-H */
	    screen.hide_cursor ();
	    return 'h';
	    break;

	  case 46: /* Alt-C */
	    screen.hide_cursor ();
	    return 'c';
	    break;

	  case 59: /* F1 */
	    help = 1;
	    break;

	  case 71: /* Home */
	    m = 0;
	    screen.gotoxy (x, y);
	    break;

	  case 72: /* Up arrow */
	    screen.hide_cursor ();
	    return 72;
	    break;

	  case 75: /* Left arrow */
	    if (m) m--;
	    screen.gotoxy (x + m, y);
	    break;

	  case 77: /* Right arrow */
	    if (m < l) m++;
	    screen.gotoxy (x + m, y);
	    break;

	  case 79: /* End */
	    m = l;
	    screen.gotoxy (x + m, y);
	    break;

	  case 80: /* Down arrow */
	    screen.hide_cursor ();
	    return 80;
	    break;

	  case 82: /* Insert */
	    insert = insert ^ 1;
	    if (insert) 
	      screen.line_cursor ();
	    else 
	      screen.block_cursor ();
	    break;

	  case 83: /* Delete */
	    if (m < l)
	    {
	      memcpy (tmp_str + m, tmp_str + m + 1, l - m);
	      screen.put (x, y, tmp_str, config.cfg.color[MISC_NORMAL]);
	      screen.put (x + l - 1, y, 0x20, config.cfg.color[MISC_NORMAL]);
	      screen.refresh ();
	      l--;
	    }
	    break;
	}
	break;

      case 1: /* Ctrl-A */
	m = 0;
	screen.gotoxy (x, y);
	break;

      case 4: /* Ctrl-D */
	if (m < l)
	{
	  memcpy (tmp_str + m, tmp_str + m + 1, l - m);
	  screen.put (x, y, tmp_str, config.cfg.color[MISC_NORMAL]);
	  screen.put (x + l - 1, y, 0x20, config.cfg.color[MISC_NORMAL]);
	  screen.refresh ();
	  l--;
	}
	break;

      case 5: /* Ctrl-E */
	m = l;
	screen.gotoxy (x + m, y); 
	break;

      case 8: /* Backspace; Ctrl-H */
	if (m)
	{
	  if (m != l) memcpy (tmp_str + m - 1, tmp_str + m, l - m);
	  tmp_str[l - 1] = 0;
	  l--;
	  m--;
	  screen.put (x, y, tmp_str, config.cfg.color[MISC_NORMAL]);
	  screen.put (x + l, y, 0x20, config.cfg.color[MISC_NORMAL]);
	  screen.refresh ();
	  screen.gotoxy (x + m, y);
	}
	break;

      case 11: /* Ctrl-K */
	memset (tmp_str + m, 0x20, l - m);
	screen.put (x, y, tmp_str, config.cfg.color[MISC_NORMAL]);
	screen.refresh ();
	memset (tmp_str + m, l - m, 0);
	l = m;
	break;

      case 13: /* Enter */
	break;

      case 27: /* Escape */
	break;

      default:
	if (l < max_len)
	{
	  if (m < l) 
	  {
	    if (insert)
	    {
	      tmp_str2 = new char[256];
	      memset (tmp_str2, 0, 256);
	      strcpy (tmp_str2, tmp_str + m);
	      strcpy (tmp_str + m + 1, tmp_str2);
	      delete tmp_str2;
	    }
	  }
	  tmp_str[m] = c;
	  if (insert)
	    l++;
	  m++;

	  screen.put (x, y, tmp_str, config.cfg.color[MISC_NORMAL]);
	  screen.refresh ();
	  screen.gotoxy (x + m, y);
	}
    }
  } while (!((c == 13) || (c == 27) || (help == 1)));

  screen.hide_cursor ();
  delete old_str;
  return c;
}

/* Edit the message header */

char EDITOR::edit_header (msghdr_type *msghdr)
{
  char done = FALSE;
  int i;
  byte where = 2, c = 0;
  char tmpstr[256];

  screen.save ();
  screen.clear ();

  screen.color = config.cfg.color[MISC_NORMAL];

  for (i = 0; i < screen.x_max; i++)
    tmpstr[i] = ' ';
  tmpstr[screen.x_max] = 0;

  for (i = 1; i < 4; i++)
    screen.put (1, i, tmpstr);

  if (config.cfg.separating_line)
    screen.hline (4);

  memset (tmpstr, 0, 256);
 
  screen.put (1, 1, "Sender    :");
//  screen.put (1, 2, " ÀÄÄ Address ");
  screen.put (1, 2, "Recipient :", config.cfg.color[MISC_CHOSEN]);
//  screen.put (1, 4, " ÀÄÄ Address ");
  screen.put (1, 3, "Subject   :");
  screen.color = config.cfg.color[MISC_WINDOW];

  screen.put (1, screen.y_max, " Area: ", config.cfg.color[MISC_STATUS]);
  screen.put (8, screen.y_max, config.area[msghdr->folder]->comment, config.cfg.color[MISC_STATUS]);
  for (c = strlen (config.area[msghdr->folder]->comment) + 8; c < screen.x_max; c++)
    screen.put (c, screen.y_max, 32, config.cfg.color[MISC_STATUS]);

  screen.put (13, 1, msghdr->sender, config.cfg.color[MISC_NORMAL]);
  if (msghdr->folder == 1)
  {
    screen.put (13 + strlen ((char *) msghdr->sender), 1, ", ", config.cfg.color[MISC_NORMAL]);
    screen.put (15 + strlen ((char *) msghdr->sender), 1, binToStrAddr (msghdr->sender_addr), config.cfg.color[MISC_NORMAL]);
  }
  screen.put (13, 2, msghdr->recipient, config.cfg.color[MISC_NORMAL]);
  if (msghdr->folder == 1 && msghdr->recipient_addr.zone)
  {
    screen.put (13 + strlen ((char *) msghdr->recipient), 2, ", ", config.cfg.color[MISC_NORMAL]);
    screen.put (15 + strlen ((char *) msghdr->recipient), 2, binToStrAddr (msghdr->recipient_addr), config.cfg.color[MISC_NORMAL]);
  }
  screen.put (13, 3, msghdr->subject, config.cfg.color[MISC_NORMAL]);

  /* Flags should be shown. At least some of them. */

  if (msghdr->status_flags & FLAG_ATTACH)  /* Fileattach */
    strcat (tmpstr, "f/a ");
  if (msghdr->status_flags & FLAG_CRASH) /* Crash */
    strcat (tmpstr, "cra ");
  if (msghdr->status_flags & FLAG_HOLD) /* Hold */
    strcat (tmpstr, "hld ");
  screen.put (screen.x_max - strlen (tmpstr) + 1, 1, tmpstr, 
	      config.cfg.color[MISC_NORMAL]);
  
  do
  {
    switch (where)
    {
      case 0: /* Sender name */
	for (i = 13; i < screen.x_max; i++) screen.put (i, 1, ' ', config.cfg.color[MISC_NORMAL]);
	screen.put (1, 1, "Sender    :", config.cfg.color[MISC_CHOSEN]);
	screen.put (13, 1, msghdr->sender, config.cfg.color[MISC_NORMAL]);
	screen.refresh ();
	c = read_string (13, 1, (char *) msghdr->sender, 35);

	/* Redraw line */

	screen.put (13, 1, msghdr->sender, config.cfg.color[MISC_NORMAL]);
	if (msghdr->folder == 1)
	{
	  screen.put (13 + strlen ((char *) msghdr->sender), 1, ", ", config.cfg.color[MISC_NORMAL]);
	  screen.put (15 + strlen ((char *) msghdr->sender), 1, binToStrAddr (msghdr->sender_addr), config.cfg.color[MISC_NORMAL]);
	}
	
	switch (c)
	{
	  case 13:
	  case 80:
	    if (msghdr->folder == 1)
	      where = 1;
	    else
	      where = 2; 
	    break;
	  case 72:
	    where = 4; break;
	  case 27:
	    done = ABORT; break;
	  default:
	    special_char (c, msghdr);
	    break;
	}
	screen.put (1, 1, "Sender    :", config.cfg.color[MISC_NORMAL]);
	break;

      case 1: /* Sender address */
	memset (tmpstr, 0, 256);
	strcpy (tmpstr, binToStrAddr (msghdr->sender_addr));
	screen.put (1, 1, "Sender    :", config.cfg.color[MISC_CHOSEN]);
	screen.refresh ();

	c = read_string (15 + strlen ((char *) msghdr->sender), 1, (char *) tmpstr, 70);
	msghdr->sender_addr = strToBinAddr (tmpstr);

	switch (c)
	{
	  case 13:
	  case 80:
	    where = 2; break;
	  case 72:
	    where = 0; break;
	  case 27:
	    done = ABORT; break;
	  default:
	    special_char (c, msghdr);
	    break;
	}
	screen.put (1, 1, "Sender    :", config.cfg.color[MISC_NORMAL]);
	break;

      case 2: /* Recipient */
	for (i = 13; i < screen.x_max; i++) screen.put (i, 2, ' ', config.cfg.color[MISC_NORMAL]);
	screen.put (1, 2, "Recipient :", config.cfg.color[MISC_CHOSEN]);
	screen.put (13, 2, msghdr->recipient, config.cfg.color[MISC_NORMAL]);
	screen.refresh ();
	c = read_string (13, 2, (char *) msghdr->recipient, 35);

	/* Redraw line */

	screen.put (13, 2, msghdr->recipient, config.cfg.color[MISC_NORMAL]);
	if (msghdr->folder == 1)
	{
	  screen.put (13 + strlen ((char *) msghdr->recipient), 2, ", ", config.cfg.color[MISC_NORMAL]);
	  if (msghdr->recipient_addr.zone != 0)
	    screen.put (15 + strlen ((char *) msghdr->recipient), 2, binToStrAddr (msghdr->recipient_addr), config.cfg.color[MISC_NORMAL]);
	}

	switch (c)
	{
	  case 13:
	  case 80:
	    if (msghdr->folder == 1)
	      where = 3;
	    else
	      where = 4; 
	    break;
	  case 72:
	    if (msghdr->folder == 1)
	      where = 1;
	    else
	      where = 0; 
	    break;
	  case 27:
	    done = ABORT; break;
	  default:
	    special_char (c, msghdr);
	    break;
	}
	screen.put (1, 2, "Recipient :", config.cfg.color[MISC_NORMAL]);
	break;

      case 3: /* Recipient address */
	memset (tmpstr, 0, 256);
	if (msghdr->recipient_addr.zone)
	  strcpy (tmpstr, binToStrAddr (msghdr->recipient_addr));
	screen.put (1, 2, "Recipient :", config.cfg.color[MISC_CHOSEN]);
	screen.refresh ();

	c = read_string (15 + strlen ((char *) msghdr->recipient), 2, (char *) tmpstr, 70);
	msghdr->recipient_addr = strToBinAddr (tmpstr);

	switch (c)
	{
	  case 13:
	  case 80:
	    where = 4; break;
	  case 72:
	    where = 2; break;
	  case 27:
	    done = ABORT; break;
	  default:
	    special_char (c, msghdr);
	    break;
	}
	screen.put (1, 2, "Recipient :", config.cfg.color[MISC_NORMAL]);
	break;

      case 4: /* Subject */
	screen.put (1, 3, "Subject   :", config.cfg.color[MISC_CHOSEN]);
	screen.refresh ();

	c = read_string (13, 3, (char *) msghdr->subject, 72);
	switch (c)
	{
	  case 13:
	    if ((msghdr->folder == 1 && msghdr->recipient_addr.zone) || msghdr->folder != 1)
	      done = TRUE; 
	    break;
	  case 72: 
	    if (msghdr->folder == 1)
	      where = 3;
	    else
	      where = 2; 
	    break;
	  case 80:
	    where = 0; break;
	  case 27:
	    done = ABORT; break;
	  default:
	    special_char (c, msghdr);
	    break;
	}
	screen.put (1, 3, "Subject   :", config.cfg.color[MISC_NORMAL]);
	break;
    }
  } while (!done);
  screen.restore ();
  return done;
}

/* Deal with some special things */

void EDITOR::special_char (byte c, msghdr_type *msghdr)
{
  char tmpstr[256];
  int i;
  
  switch (c) 
  {
    case 'f':
      if (msghdr->status_flags | FLAG_ATTACH)
	msghdr->status_flags ^= FLAG_ATTACH;
      else
      {
	fattach (msghdr);
	screen.put (17, 5, msghdr->subject, config.cfg.color[MISC_NORMAL]);
      }
      break;
    case 'c': /* Crash */
      msghdr->status_flags ^= FLAG_CRASH;
      msghdr->status_flags &= ~FLAG_HOLD;
      break;
    case 'h': /* Hold */
      msghdr->status_flags ^= FLAG_HOLD;
      msghdr->status_flags &= ~FLAG_CRASH;
      break;
  }

  /* Flags should be shown. At least some of them. */

  for (i = strlen ((char *) msghdr->sender); i < screen.x_max; i++) 
    screen.put(i, 1, ' ', config.cfg.color[MISC_NORMAL]);
  tmpstr[0] = 0;
  if (msghdr->status_flags & FLAG_ATTACH) /* Fileattach */
    strcat (tmpstr, "f/a ");
  if (msghdr->status_flags & FLAG_CRASH) /* Crash */
    strcat (tmpstr, "cra ");
  if (msghdr->status_flags & FLAG_HOLD) /* Hold */
    strcat (tmpstr, "hld ");
  screen.put (screen.x_max - strlen (tmpstr) + 1, 1, tmpstr,
	      config.cfg.color[MISC_NORMAL]);
}

/* Attach a file to the message. */

void EDITOR::fattach (msghdr_type *msghdr)
{
  Window *win;
  char estr[256], filename[80] = "";
  struct stat junk;

  win = new Window (1, screen.y_max / 2, screen.x_max, 3, 
		    config.cfg.color[MISC_WINDOW],
		    config.cfg.color[MISC_TITLE],
		    "Enter filename");
  screen.refresh ();
  win->get_line (2, 1, filename, 79, TRUE);
  delete win;
  if (!filename[0]) return;
  
  if ((lstat (filename, &junk)) < 0)
  {
    sprintf (estr, "Could not open file \"%s\" (%s)",
	     filename, strerror (errno));
    ui.infobox ("Error", estr);
    return;

  }

  if ((strlen ((char *) msghdr->subject) + strlen (filename)) >= 71)
  {
    sprintf (estr, "Filename too long");
    ui.infobox ("Error", estr);
    return;
  }

  if (!strlen ((char *) msghdr->subject) && (strlen (filename) < 72))
    strcpy ((char *) msghdr->subject, filename);
  else
    sprintf ((char *) msghdr->subject, "%s %s", (char *) msghdr->subject, 
	     filename);
  msghdr->status_flags |= FLAG_ATTACH;
  return;
}

/* Show the message on the screen, starting with line `delta' */

void EDITOR::show_message (char **buffer, word delta)
{
  byte c, d;

  for (c = 0; c < screen.y_max - 1 && buffer[c + delta] != NULL; c++)
  {
    choose_color (buffer[delta + c], d);
	  
    screen.put (1, c + 1, buffer[delta + c], d);
    for (d = strlen (buffer[delta + c]) + 1; d <= screen.x_max; d++)
      screen.put (d, c + 1, ' ', config.cfg.color[EDITOR_NORMAL]);
  }

  if (c < screen.y_max - 1)
  {
    for (; c < screen.y_max - 1; c++)
      for (d = 0; d <= screen.x_max; d++)
	screen.put (d, c + 1, ' ', config.cfg.color[EDITOR_NORMAL]);
  }
  screen.refresh ();
}

/* Show a status line at the bottom of the screen */

void EDITOR::show_statusline (byte x, word y, msghdr_type *msghdr)
{
  char tmpstr[256];
  byte c;

  screen.hide_cursor ();
  sprintf (tmpstr, " Writing to \"%s\" in %s (%s)", msghdr->recipient,
	   config.area[msghdr->folder]->name,
	   config.area[msghdr->folder]->comment);

  /* If the line is to long for the screen, cut it in an elegant way */

  if (strlen (tmpstr) > (screen.x_max - 22))
  {
    strcpy (tmpstr + screen.x_max - 13, "...)");
  }

  screen.put (1, screen.y_max, tmpstr, config.cfg.color[MISC_STATUS]);
  c = strlen (tmpstr) + 1;
  sprintf (tmpstr, "³ %u : %u ", x + 1, y + 1);
  for (; c <= screen.x_max - strlen (tmpstr); c++)
    screen.put (c, screen.y_max, ' ', config.cfg.color[MISC_STATUS]);
  screen.put (screen.x_max - strlen (tmpstr), screen.y_max, tmpstr,
	      config.cfg.color[MISC_STATUS]);
  //  screen.gotoxy (x + 1, y + 1);
  screen.show_cursor ();
}

/* Choose which color to use for the string passed */

void EDITOR::choose_color (char *s, byte &color)
{
  if (is_quote (s))
  {
    color = config.cfg.color[EDITOR_QUOTE];
  }
  else if (is_origin (s))
  {
    color = config.cfg.color[EDITOR_ORIGIN];
  }
  else
  {
    color = config.cfg.color[EDITOR_NORMAL];
  }
}
  
/* The main editor part */

char *EDITOR::edit_message (char *text, msghdr_type *msghdr, byte flags)
{
  char **buffer = new char *[65536]; /* maximum of 65536 rows */
  char tmpstr[256], *tmpstr2;
  char done = FALSE;
  word x = 0, y = 0;
  word lines = 0;
  word delta = 0;
  byte c = 0, color = config.cfg.color[EDITOR_NORMAL];
  dword n, m = 0, position = 0, len;
  bool insert = 1;

  /* Split the message text into lines with the same width as the screen */
  // FIXME: this isn't entirely finished 

  if (text != NULL) 
  {
    for (int ull = 0; text[ull] != '\0'; ull++)
    {
      if ((byte) text[ull] == 0x8D)
      {
        text[ull] = ' ';
      }
    }
    
    /* Get the total length (in characters) of the buffer */

    len = strlen (text) - 1;
      
    while (position < len)
    {
      /* Go to the end of the current line (indicated by a \r) */

      while ((*(text + position + x) != '\r') && (x < screen.x_max - 1))
      {
	x++;
      } 
     
      /* If the length is above the maximum (it probably is if it's not a
	 quoted line, backtrace to previous space */


      if (x == screen.x_max - 1)
      {
	do
	{
	  x--;
	}
	while (x && *(text + position + x) != ' ');

	if (!x) /* The current word is too long; we have to split it. */
	{
	  keyboard.done ();
          while (TRUE);
	}
      }

      /* Is this a quoted line? If so, get the quote string (usually
	 XX>) from the current line and prepend it to the next line */
      
      if (is_quote (text + position) && (*(text + position + x) == ' '))
      {
	/* Create a new line in the buffer and copy the beginning of the line
	   to it. */
	
	buffer[lines] = new char[screen.x_max + 1];
	memset (buffer[lines], 0, screen.x_max + 1);
	strncpy (buffer[lines], text + position, x + 1);
	buffer[lines][x] = '\r';
	buffer[lines][x + 1] = 0;
	lines++;
	position += x + 1;
	x = 0;

	while ((*(text + position + x) != '\r') && (x < screen.x_max - 1))
	  x++;

	/* Create another line in the buffer and copy the quote string
           to it. */

	buffer[lines] = new char[screen.x_max + 1];
	memset (buffer[lines], 0, screen.x_max + 1);

	// FIXME: extremely ugly way of going round the problem. Lines too
	// long should be split instead of just fooled around like this!!
	if (!(x + strlen (get_quote (buffer[lines - 1])) > screen.x_max - 1))
	{
	  strcpy (buffer[lines], get_quote (buffer[lines - 1]));
	  strcat (buffer[lines], " ");
	}

	/* Finally, copy the rest of the line to the buffer. */

	strncpy (buffer[lines] + strlen (buffer[lines]), text + position, x + 1);
	if (buffer[lines][strlen (buffer[lines]) - 1] != '\r')
	  buffer[lines][strlen (buffer[lines])] = '\r';
	buffer[lines][strlen (buffer[lines])] = 0;

	lines++;
	position += x + 1;
	x = 0;
      }
      else /* This is not a quoted line, so just split it plain and simple */
      {
	buffer[lines] = new char[screen.x_max + 1];
	memset (buffer[lines], 0, screen.x_max + 1);
	strncpy (buffer[lines], text + position, x + 1);
	buffer[lines][x + 1] = 0;
	lines++;
	position += x + 1;
	x = 0;
      }
    }
    lines--;
  }
  else
  {
    buffer[lines] = new char[screen.x_max + 1];
    memset (buffer[lines], 0, screen.x_max + 1);
  }

  for (n = lines + 1; n < 65536; n++)
    buffer[n] = NULL;

  show_statusline (x, y + delta, msghdr);
  show_message (buffer, delta);
  screen.gotoxy (x + 1, y + 1);
  screen.show_cursor ();

  /* Main loop */

  do
  {
    c = keyboard.get ();
    switch (c)
    {
      case 0:
	c = keyboard.get ();
	switch (c)
	{
	  case 23: /* Alt-I, import a text file */
	    import_message (buffer, y + delta, lines);
	    show_message (buffer, delta);
	    break;

	  case 31: /* Alt-S, save */
	    done = TRUE;
	    break;

	  case 35: /* Alt-H, edit header */
	    edit_header (msghdr);
	    break;

	  case 49: /* Alt-N, nicify text */
	  {
	    sdword line;

	    /* Find the beginning of the current section */

	    line = y + delta;
	    //	    printf ("%d\n", line);
	    if (!is_quote (buffer[line])) /* not a quoted line */
	      break;
	    memset (tmpstr, 0, 256);
	    strcpy (tmpstr, get_quote (buffer[line]));

	    if (line)
	    {
	      do
	      {
		line--;
		//		printf ("%d\n", line);
		//keyboard.get ();
		tmpstr2 = get_quote (buffer[line]);
	      } while (line && tmpstr2 && !strcmp (tmpstr2, tmpstr));
	      if (tmpstr2 && strcmp (tmpstr2, tmpstr))
		line++;
	      else
		if (!tmpstr2)
		  line++;
	    }

	    while (line < lines && is_quote (buffer[line + 1]) && !strcmp (get_quote (buffer[line + 1]), tmpstr))
	    {
	      /* Find the maximum number of letters to copy to this line */

	      sdword max = screen.x_max - 1 - strlen (buffer[line]);

	      if (max < 0)
		max = 0;

	      //	      printf ("%d\n", max);

	      n = strlen (buffer[line + 1]) - strlen (get_quote (buffer[line + 1]));
	      if (n > max) /* We can not copy all of the line, so find a good place to split it */
	      {
		for (n = strlen (get_quote (buffer[line])) + max;
		     buffer[line + 1][n] != ' ' && n; n--);
		if (n > strlen (get_quote (buffer[line]))) n -= strlen (get_quote (buffer[line]));
		else
		  n = 0;
	      }
	      //	      printf ("%d\n", n);
	      //keyboard.get ();

	      if (n) /* If we can not copy a single letter, just skip this line */
	      {
		memcpy (&buffer[line][strlen (buffer[line]) - 1],
			&buffer[line + 1][strlen (get_quote (buffer[line]))], n);
		
		if (n != strlen (buffer[line + 1]) - strlen (get_quote (buffer[line + 1])))
		{
		  /* Not all of the line was copied */

		  memcpy (&buffer[line + 1][strlen (get_quote (buffer[line + 1]))],
			  &buffer[line + 1][n + strlen (get_quote (buffer[line + 1]))],
			  screen.x_max + 1 - n);
		  memset (buffer[line + 1] + screen.x_max + 1 - n, 0, n);
		  buffer[line][strlen (buffer[line])] = '\r';
		  buffer[line][strlen (buffer[line]) + 1] = 0;
		}		      
		else
		{
		  /* Scroll the lines below upwards */

		  for (n = line + 1; n < lines; n++)
		    memcpy (buffer[n], buffer[n + 1], screen.x_max + 1);
		  delete buffer[lines];
		  buffer[lines] = NULL;
		  lines--;
		  line--;
		}
	      }
	      line++;
	      show_message (buffer, delta);
	      //	      keyboard.get ();
	    }
	    show_message (buffer, delta);
	    break;
	  }
	  case 59: /* F1, help */
	    help ();
	    break;
	  case 71: /* Home, beginning */
	    x = 0;
	    screen.gotoxy (x + 1, y + 1);
	    show_statusline (x, y + delta, msghdr);
	    screen.refresh ();
	    break;
	  case 72: /* Up Arrow */
	    if (y) 
	      y--;
	    else
	      if (delta)
	      {
		delta--;
		show_message (buffer, delta);
	      }
	    if (x > strlen (buffer[y + delta]))
	      x = strlen (buffer[y + delta]);
	    screen.gotoxy (x + 1, y + 1);
	    show_statusline (x, y + delta, msghdr);
	    screen.refresh ();
	    break;
	  case 73: /* Page Up */
	    if (delta > screen.y_max - 1)
	    {
	      delta -= screen.y_max - 1;
	      show_message (buffer, delta);
	    }
	    else
	      delta = y = 0;
	    screen.gotoxy (x + 1, y + 1);
	    show_statusline (x, y + delta, msghdr);
	    screen.refresh ();
	    break;
	  case 75: /* Left Arrow */
	    if (x)
	      x--;
	    screen.gotoxy (x + 1, y + 1);
	    show_statusline (x, y + delta, msghdr);
	    screen.refresh ();
	    break;
	  case 77: /* Right Arrow */
	    if (x < strlen (buffer[y + delta]) && buffer[y + delta][x] != '\r') 
	      x++;
	    screen.gotoxy (x + 1, y + 1);
	    show_statusline (x, y + delta, msghdr);
	    screen.refresh ();
	    break;
	  case 79: /* End */
	    x = strlen (buffer[y + delta]);
	    if (buffer[y + delta][x - 1] == '\r') 
	      x--;
	    screen.gotoxy (x + 1, y + 1);
	    show_statusline (x, y + delta, msghdr);
	    screen.refresh ();
	    break;
	  case 80: /* Down Arrow */
	    if (y + delta < lines) 
	    {
	      y++;
	      if (y > screen.y_max - 2)
	      {
		y--;
		delta++;
		show_message (buffer, delta);
	      }
	      if (x > strlen (buffer[y + delta]))
		x = strlen (buffer[y + delta]);
	    }
	    screen.gotoxy (x + 1, y + 1);
	    show_statusline (x, y + delta, msghdr);
	    screen.refresh ();
	    break;
	  case 81: /* Page Down */
	    if (y + delta + screen.y_max - 1 < lines)
	    {
	      delta += screen.y_max - 1;
	      show_message (buffer, delta);
	    }
	    else
	    {
	      if (lines > screen.y_max - 2)
	      {
		y = screen.y_max - 2;
		delta = lines - y;
	      }
	      else
		y = lines;
	      show_message (buffer, delta);
	    }
	    screen.gotoxy (x + 1, y + 1);
	    show_statusline (x, y + delta, msghdr);
	    screen.refresh ();
	    break;
	  case 82: /* Insert */
	    insert ^= 1;
	    insert ? screen.line_cursor () : screen.block_cursor ();
	    break;
	  case 83: /* Delete */
	    if (x == strlen (buffer[y + delta]) || buffer[y + delta][x] == '\r')
	    {
	      if (y < lines)
	      {
		n = strlen (buffer[y + delta + 1]);
		if (n > screen.x_max - 1 - x) /* we can not copy all of the line */
		  for (m = screen.x_max - 1 - x; buffer[y + delta + 1][m] != ' ' && m; m--);
		else
		  m = n;
		memcpy (&buffer[y + delta][x], &buffer[y + delta + 1][0], m);

		if (!m && buffer[y + delta][x] == '\r') /* nothing was copied, but remove the CR-marker */
		  buffer[y + delta][x] = 0;

		if (m != n) /* not all of the line was copied */
		{
		  memcpy (&buffer[y + delta + 1][0], &buffer[y + delta + 1][m], screen.x_max + 1 - m);
		  memset (buffer[y + delta + 1] + screen.x_max + 1 - m, 0, m);
		}		      
		else /* scroll the lines below upwards */
		{
		  for (n = y + delta + 1; n < lines; n++)
		    memcpy (buffer[n], buffer[n + 1], screen.x_max + 1);
		  delete buffer[lines];
		  buffer[lines] = NULL;
		  lines--;
		}
		show_message (buffer, delta);
	      }
	    }
	    else
	    {
	      memcpy (&buffer[y + delta][x], &buffer[y + delta][x + 1], strlen (buffer[y + delta]) - x);
	      buffer[y + delta][strlen (buffer[y + delta])] = 0;
	      choose_color (buffer[y + delta], color);
	      screen.put (1, y + 1, buffer[y + delta], color);
	      screen.put (strlen (buffer[y + delta]) + 1, y + 1, ' ', color);
	      screen.refresh ();
	    }
	    break;
	  case 117: /* Ctrl-Home */
	    y = delta = 0;
	    show_message (buffer, delta);
	    screen.gotoxy (x + 1, y + 1);
	    show_statusline (x, y + delta, msghdr);
	    break;
	  case 119: /* Ctrl-End */
	    if (lines > screen.y_max - 2)
	    {
	      y = screen.y_max - 2;
	      delta = lines - y;
	    }
	    else
	      y = lines;
	    show_message (buffer, delta);
	    screen.gotoxy (x + 1, y + 1);
	    show_statusline (x, y + delta, msghdr);
	    break;
	}
	break;
      case 1: /* Ctrl-A */
	x = 0;
	screen.gotoxy (x + 1, y + 1);
	show_statusline (x, y + delta, msghdr);
	break;
      case 5: /* Ctrl-E */
	x = strlen (buffer[y + delta]);
	if (buffer[y + delta][x - 1] == '\r') 
	  x--;
	screen.gotoxy (x + 1, y + 1);
	show_statusline (x, y + delta, msghdr);
	break;
      case 8: /* Backspace */
	if (x)
	{
	  x--;
	  memcpy (&buffer[y + delta][x], &buffer[y + delta][x + 1],
		  strlen (buffer[y + delta]) - x);
	  buffer[y + delta][strlen (buffer[y + delta])] = 0;
	  choose_color (buffer[y + delta], color);
	  screen.put (x + 1, y + 1, &buffer[y + delta][x], color);
	  screen.put (strlen (buffer[y + delta]) + 1, y + 1, ' ', color);
	  screen.refresh ();
	  screen.gotoxy (x + 1, y + 1);
	  show_statusline (x, y + delta, msghdr);
	}
	break;
      case 9: /* Tab */
	if (insert && x != strlen (buffer[y + delta]))
	{
	  memset (tmpstr, 0, 256);
	  memcpy (tmpstr, &buffer[y + delta][x], strlen (buffer[y + delta]) - x);
	  memcpy (&buffer[y + delta][x + 2], tmpstr, strlen (tmpstr));
	}

	buffer[y + delta][x] = ' ';
	buffer[y + delta][x + 1] = ' ';
	x += 2;
	show_message (buffer, delta);
	screen.gotoxy (x + 1, y + 1);
	show_statusline (x, y + delta, msghdr);
	break;
      case 11: /* Ctrl-K */
	break;
      case 13: /* Enter */
	if (y + delta != lines) /* scroll the lines below this one */
	{
	  lines++;
	  buffer[lines] = new char[screen.x_max + 1];
	  memset (buffer[lines], 0, screen.x_max + 1);
	      
	  for (n = lines - 1; n > y + delta; n--)
	  {
	    memset (buffer[n + 1], 0, screen.x_max + 1);
	    memcpy (buffer[n + 1], buffer[n], screen.x_max + 1);
	  }
	}
	else
	{
	  lines++;
	  buffer[lines] = new char[screen.x_max + 1];
	  memset (buffer[lines], 0, screen.x_max + 1);
	}

	memset (buffer[y + delta + 1], 0, screen.x_max);
	if (is_quote (buffer[y + delta]) && !is_quote (buffer[y + delta] + x))
	  strcpy (buffer[y + delta + 1], get_quote (buffer[y + delta]));
	memcpy (buffer[y + delta + 1] + strlen (buffer[y + delta + 1]), &buffer[y + delta][x], screen.x_max - x + 1);
	memset (&buffer[y + delta][x], 0, screen.x_max - x + 1);
	buffer[y + delta][x] = '\r';
	y++;
	  
	if (y > screen.y_max - 2)
	{
	  delta++;
	  y--;
	}

	x = 0;
	show_message (buffer, delta);
	screen.gotoxy (x + 1, y + 1);
	show_statusline (x, y + delta, msghdr);
	break;
      case 25: /* Ctrl-Y */
	if (lines)
	{
	  if (y + delta == lines) /* last line */
	  {
	    memset (buffer[lines], 0, screen.x_max + 1);
	    buffer[lines][0] = '\r';
	    x = 0;
	  }
	  else 
	  {
	    for (n = y + delta; n < lines; n++)
	    {
	      memset (buffer[n], 0, screen.x_max + 1);
	      memcpy (buffer[n], buffer[n + 1], screen.x_max + 1);
	    }

	    if (x > strlen (buffer[y + delta]))
	      x = strlen (buffer[y + delta]);

	    delete buffer[lines];
	    buffer[lines] = NULL;
	    lines--;
	  }
	      
	  screen.gotoxy (x + 1, y + 1);
	  show_message (buffer, delta);
	}
	break;
      case 27: /* Escape */
	screen.hide_cursor ();
	if (ui.yesno ("Abort message?") == YES)
	  done = ABORT;
	screen.show_cursor ();
	break;
      default: /* All other characters are to be treated as text */
	if (x == screen.x_max - 1) /* we're at the end of the line */
	{
	  n = x;
	  while (buffer[y + delta][x] != ' ') /* find the beginning of the current word */
	    x--;

	  if (y + delta != lines) /* scroll the lines below down */
	  {
	    buffer[lines + 1] = new char[screen.x_max + 1];
	    lines++;
	    memset (buffer[lines], 0, screen.x_max + 1);

	    for (m = lines - 1; m > y + delta; m--)
	    {
	      memset (buffer[m + 1], 0, screen.x_max + 1);
	      memcpy (buffer[m + 1], buffer[m], screen.x_max + 1);
	    }
	  }
	  else /* no need to scroll, since we are on the last line */
	  {
	    buffer[lines + 1] = new char[screen.x_max + 1];
	    lines++;
	    memset (buffer[lines], 0, screen.x_max + 1);
	  }
	  y++;

	  if (y > screen.y_max - 2)
	  {
	    y--;
	    delta++;
	  }
		
	  memset (buffer[y + delta], 0, screen.x_max + 1);
	  memcpy (buffer[y + delta], buffer[y + delta - 1] + x + 1, n - x - 1);
	  memcpy (buffer[y + delta] + strlen (buffer[y + delta]), buffer[y + delta - 1] + n, screen.x_max - n);
	  memset (buffer[y + delta - 1] + x, 0, n - x + 1);
	  x = n - x - 1;

	  show_message (buffer, delta);
	}

	if (insert && x != strlen (buffer[y + delta])) /* Scroll the rest of the line */
	{
	  if (strlen (buffer[y + delta]) == screen.x_max) /* Wrap the line */
	  {
	  }
	  else
	  {
	    memset (tmpstr, 0, 256);
	    memcpy (tmpstr, &buffer[y + delta][x], strlen (buffer[y + delta]) - x);
	    memcpy (&buffer[y + delta][x + 1], tmpstr, strlen (tmpstr));
	  }
	}
	else /* No need to scroll, we're at the end of the line */
	{
	}

	choose_color (buffer[y + delta], color);

	buffer[y + delta][x] = c;
	screen.put (1, y + 1, buffer[y + delta], color);
	x++;
	screen.refresh ();
	screen.gotoxy (x + 1, y + 1);
	show_statusline (x, y + delta, msghdr);
	break;  
    }
  } while (!done);
  
  screen.hide_cursor ();
  screen.line_cursor ();

  if (done == ABORT)
  {
    for (n = 0; buffer[n] != NULL; n++)
    {
      delete buffer[n];
      buffer[n] = NULL;
    }
      
    delete buffer;
    buffer = NULL;
    return NULL;
  }

  /* Concatenate all lines into one buffer */

  if (msgbuf != NULL) 
  {
    delete msgbuf;
    msgbuf = NULL;
  }

  for (m = 0, n = 0; buffer[m] != NULL; m++)
    n += strlen (buffer[m]);
  msgbuf = new char[n + 1 + lines];
  memset (msgbuf, 0, n + 1 + lines);

  for (m = 0; buffer[m] != NULL; m++)
  {
    strcat (msgbuf, buffer[m]);
    if (buffer[m][strlen (buffer[m]) - 1] != '\r') 
      strcat (msgbuf, " ");
  }

  for (n = 0; buffer[n] != NULL; n++)
  {
    delete buffer[n];
    buffer[n] = NULL;
  }
 
  delete buffer;
  buffer = NULL;

  return msgbuf;
}

/* Import a text file to the current message */

void EDITOR::import_message (char **buffer, word delta, word &lines)
{
  Window *win;
  char filename[80] = "";
  char estr[256], tmp[256];
  int i;
  FILE *file;

  getcwd (tmp, 256);
  if (getenv ("HOME")) chdir (getenv ("HOME"));

  win = new Window (1, screen.y_max / 2, screen.x_max, 3,
                    config.cfg.color[MISC_WINDOW],
                    config.cfg.color[MISC_TITLE],
                    "Enter filename");
  screen.refresh ();
  win->get_line (2, 1, filename, 79, TRUE);
  delete win;
  if (!filename[0]) return;

  if (!(file = fopen (filename, "r"))) 
  {
    sprintf (estr, "Couldn't open \"%s\" (%s)",
	     filename, strerror (errno));
    ui.infobox ("Error", estr);
    return;
  }

  while(!feof(file)) 
  {
    lines++;
    if (delta < (lines - 1)) 
    {
      for (i = lines; i > delta; i--) buffer[i] = buffer[i - 1];
      buffer[delta] = new char[screen.x_max + 1];
      memset (buffer[delta], 0, screen.x_max + 1);
    } 
    else 
    {
      buffer[lines] = new char[screen.x_max + 1];
      memset (buffer[lines], 0, screen.x_max + 1);
    }
    fgets (buffer[delta], screen.x_max, file);
    for (i = 0; i < screen.x_max; i++)
      if (buffer[delta][i] == '\n') buffer[delta][i] = '\r';
#ifdef __linux__
    convert_charset ((uchar *) buffer[delta], 0, 1);
#endif
    delta++;
  }
  
  chdir(tmp);
  fclose(file);
}

void EDITOR::help (void)
{
  Window *win;
  byte c;

  screen.hide_cursor ();
  win = new Window (0, 0, 52, 17, config.cfg.color[MISC_WINDOW],
                    config.cfg.color[MISC_TITLE], "Help");
  win->color = config.cfg.color[MISC_NORMAL];
  win->put ("Up           Previous line");
  win->put ("Down         Next line");
  win->put ("Left Arrow   Go to the previous message");
  win->put ("Right Arrow  Go to the following message");
  win->put ("Home         Beginning of line");
  win->put ("End          End of line");
  win->put ("Ctrl-Home    Beginning of message");
  win->put ("Ctrl-End     End of message");
  win->put ("Insert       Toggle insert/overwrite");
  win->put ("Delete       Delete character at cursor position");
  win->put ("Esc          Abort message");
  win->put ("Alt-H        Edit message header");
  win->put ("Alt-I        Import file");
  win->put ("Alt-N        Nicify quoted text");
  win->put ("Alt-S        Save message");
  screen.refresh ();

  do 
  {
    c = keyboard.get ();
  } while (c != 13 && c != 27);
  screen.show_cursor ();
  delete win;
}
