/* configuration.cc - configuration part of tanstaafl */
/* Copyright 1998-1999 Per Lundberg */
/* Copyright 1999 Henrik Hallin */

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

#include <errno.h>
#include <memory.h>
#include <fstream.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>

char *quote_type[3] =
{
  ">  ", "xy>", "XY>"
};

char *yes_no[2] =
{
  "No ",
  "Yes"
};

#define SCHEMES 7

/* Don't add schemes manually; use the make_scheme program supplied with
   the distribution. And please, send good schemes to me so that other users
   may benefit from them too. */

byte color_scheme[SCHEMES][64] =
{
  /* Default */
  { 
    0x07, /* READER_NORMAL */
    0x03, /* READER_QUOTE */
    0x03, /* READER_KLUDGE */
    0x09, /* READER_ORIGIN */
    0x17, /* READER_HEADER */
    0x1F, /* READER_PERSONAL */
    0, 0, 0, 0, 0,0, 0, 0, 0, 0,
    
    0x07, /* EDITOR_NORMAL */
    0x03, /* EDITOR_QUOTE */
    0x09, /* EDITOR_ORIGIN */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    
    0x1F, /* MISC_STATUS */
    0x03, /* MISC_WINDOW */
    0x0B, /* MISC_TITLE */
    0x07, /* MISC_NORMAL */
    0x0F, /* MISC_HIGHLIGHT */
    0x1F, /* MISC_CHOSEN */
    0x01, /* MISC_BACKGROUND */
    0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  /* Nostalgia */
  {
    2,
    10,
    2,
    2,
    2,
    10,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    2,
    10,
    2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    32,
    2,
    10,
    2,
    10,
    160,
    2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  /* Ice */
  {
    3,
    11,
    9,
    3,
    48,
    63,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3,
    11,
    3,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    48,
    3,
    11,
    3,
    11,
    176,
    9,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  /* Ketchup and mustard */
  {
    4,
    12,
    6,
    6,
    96,
    104,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4,
    12,
    6,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    96,
    4,
    12,
    4,
    14,
    96,
    4,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  /* Monochrome */
  {
    7,
    15,
    7,
    7,
    112,
    127,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    7,
    15,
    7,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    112,
    7,
    15,
    7,
    15,
    112,
    7,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  /* Pink Panther */
  {
    5,
    13,
    7,
    7,
    80,
    87,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    5,
    13,
    7,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    80,
    80,
    95,
    87,
    95,
    31,
    5,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  /* Blue and White */
  {
    7,
    15,
    8,
    15,
    23,
    31,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    7,
    15,
    15,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    31,
    31,
    31,
    23,
    31,
    113,
    8,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
};

char *scheme_description[SCHEMES] =
{
  "Default",
  "Nostalgia",
  "Ice",
  "Ketchup and mustard",
  "Monochrome",
  "Pink Panther",
  "Blue and White",
};

#ifdef __MSDOS__
char TANSTACFG[128] = "tanstafl.cfg"; 
#else
char TANSTACFG[128] = "tanstaafl.cfg"; 
#endif

#ifdef __MSDOS__
char TANSTAREAS[128] = "tanstafl.ar";
#else
char TANSTAREAS[128] = "tanstaafl.areas";
#endif

char *color_description[64] = 
{
  "Normal text",
  "Quoted text",
  "Kludges",
  "Origin and tear line",
  "Message header",
  "Personal message",
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  "Normal text",
  "Quoted text",
  "Origin and tear line",
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  "Status lines",
  "Window borders",
  "Window titles",
  "Normal window text",
  "Highlighted text",
  "Chosen alternatives",
  "Background"
};

void CONFIG::read_address (uchar x, uchar y, address_type *address)
{
  address_type a = *address;
  char *tmpstr;

  if (a.zone) tmpstr = binToStrAddr (a);
  else
  {
    tmpstr = new char[256];
    memset (tmpstr, 0, 256);
  }

  ui.get_line (x, y, tmpstr, 5 * 4 + 3, TRUE);
  a = strToBinAddr (tmpstr);
 
  *address = a;
  delete tmpstr;
}

/* Configure the AKAs used by this system */

void CONFIG::configure_akas (void)
{
  LISTBOX *list;
  bool done = FALSE;
  char choice = 1;

  list = new LISTBOX (0, 0, 35, 0, "AKA Setup");
  list->insert ("Main AKA", "Enter your main FTN address");
  list->insert ("Address 2", "Enter a secondary address");
  list->insert ("Address 3", (char *) NULL);
  list->insert ("Address 4", (char *) NULL);
  list->insert ("Address 5", (char *) NULL);
  list->insert ("Address 6", (char *) NULL);
  list->insert ("Address 7", (char *) NULL);
  list->insert ("Address 8", (char *) NULL);
  list->insert ("Address 9", (char *) NULL);
  list->insert ("Address 10", "Enter a secondary address");
  if (cfg.aka[0].zone) list->insert_data (binToStrAddr (cfg.aka[0]), 15, 1);
  if (cfg.aka[1].zone) list->insert_data (binToStrAddr (cfg.aka[1]), 15, 2);
  if (cfg.aka[2].zone) list->insert_data (binToStrAddr (cfg.aka[2]), 15, 3);
  if (cfg.aka[3].zone) list->insert_data (binToStrAddr (cfg.aka[3]), 15, 4);
  if (cfg.aka[4].zone) list->insert_data (binToStrAddr (cfg.aka[4]), 15, 5);
  if (cfg.aka[5].zone) list->insert_data (binToStrAddr (cfg.aka[5]), 15, 6);
  if (cfg.aka[6].zone) list->insert_data (binToStrAddr (cfg.aka[6]), 15, 7);
  if (cfg.aka[7].zone) list->insert_data (binToStrAddr (cfg.aka[7]), 15, 8);
  if (cfg.aka[8].zone) list->insert_data (binToStrAddr (cfg.aka[8]), 15, 9);
  if (cfg.aka[9].zone) list->insert_data (binToStrAddr (cfg.aka[9]), 15, 10);

  do
  {
    choice = list->play (choice);
    switch (choice)
    {
      case ABORT: 
	done = TRUE;
	break;
      case 1 ... 10:
	read_address (list->win->x1 + 15, list->win->y1 + choice, &cfg.aka[choice - 1]);
	if (!cfg.uplink[choice - 1].zone)
	{
	  cfg.uplink[choice - 1] = cfg.aka[choice - 1];
	  cfg.uplink[choice - 1].point = 0;
	}
	break;
    }
  } while (!done);
  delete (list);
}

void CONFIG::edit_uplink (Window *win, int which)
{
  read_address (win->x1 + 15, win->y1 + 1 + which, &cfg.uplink[which]);
}

/* Configure uplinks of this system */

void CONFIG::configure_uplinks (void)
{
  LISTBOX *list;
  bool done = FALSE;
  char choice = 1;
  byte c;

  list = new LISTBOX (0, 0, 35, 0, "AKA Setup");
  list->insert ("Main uplink", "Enter your main uplink address");
  list->insert ("Uplink 2", "Enter a secondary uplink");
  list->insert ("Uplink 3", (char *) NULL);
  list->insert ("Uplink 4", (char *) NULL);
  list->insert ("Uplink 5", (char *) NULL);
  list->insert ("Uplink 6", (char *) NULL);
  list->insert ("Uplink 7", (char *) NULL);
  list->insert ("Uplink 8", (char *) NULL);
  list->insert ("Uplink 9", (char *) NULL);
  list->insert ("Uplink 10", "Enter a secondary uplink");

  for (c = 0; c < 10; c++)
    if (cfg.uplink[c].zone) list->insert_data (binToStrAddr (cfg.uplink[c]), 15, c + 1);

  do
  {
    choice = list->play (choice);
    switch (choice)
    {
      case ABORT: 
	done = TRUE;
	break;
      case 1 ... 10:
	edit_uplink (list->win, choice - 1);
	break;
    }
  } while (!done);
  delete (list);
}

/* Configure passwords */

void CONFIG::configure_passwords (void)
{
  LISTBOX *list;
  bool done = FALSE;
  char choice = 1;
  char tmpstr[256];
  byte c;

  list = new LISTBOX (0, 0, 30, 0, "Passwords");
  list->insert ("Main password", "Enter your password for your primary uplink");
  list->insert_data ("********", 20, 1);

  for (c = 1; c < 10 && cfg.uplink[c].zone; c++)
  {
    sprintf (tmpstr, "Password for uplink %s", binToStrAddr (cfg.uplink[c]));
    printf ("%s\n", tmpstr);
    keyboard.get ();
    //    list->insert ("Other password", string (tmpstr));
    list->insert_data ("********", 20, c + 1);
  }

  do
  {
    choice = list->play (choice);
    switch (choice)
    {
      case ABORT: 
	done = TRUE;
	break;
      case 1 ... 10:
	list->win->put (20, choice, "ull!" /*cfg.password[choice - 1]*/, cfg.color[MISC_NORMAL]);
	list->win->get_line (20, choice, cfg.password[choice - 1], 8, 1);
	list->win->put (20, choice, "********", cfg.color[MISC_NORMAL]);
	break;
    }
  } while (!done);
  delete (list);
}

/* Configure phone/IP numbers */

void CONFIG::configure_phone_numbers (void)
{
  LISTBOX *list;
  bool done = FALSE;
  char choice = 1;

#ifdef MAILER_MODEM
  char tmpstr[256];

  list = new LISTBOX (0, 0, 30, 0, "Phone numbers");

  for (byte c = 0; c < 10; c++)
  {
    sprintf (tmpstr, "Enter the phone number of uplink %d", c + 1);
    list->insert ("Phone number", tmpstr);
    list->insert_data (cfg.phone[c], 20, c + 1);
  }
#elif MAILER_IP
  list = new LISTBOX (0, 0, 70, 0, "Hostnames");

  for (c = 0; c < 10; c++)
  {
    sprintf (tmpstr, "Enter the hostname of uplink %d", c + 1);
    list->insert ("Hostname", tmpstr);
    list->insert_data (cfg.phone[c], 20, c + 1);
  }
#elif MAILER_EXTERNAL
  return;
#endif

  do
  {
    choice = list->play (choice);
    switch (choice)
    {
      case ABORT: 
	done = TRUE;
	break;
      case 1 ... 10:
	list->win->get_line (20, choice, cfg.phone[choice - 1], 40, 1);
	break;
    }
  } while (!done);
  delete (list);
}

/* Configure site information */

void CONFIG::configure_site_info (void)
{
  LISTBOX *list;
  char choice = 1;
  bool done = FALSE;

  list = new LISTBOX (0, 0, 55, 0, "Site Information");
  list->insert ("Name", "Your name (first and last)");
  list->insert ("Location", "Enter the location of your point");
  list->insert ("System name", "Enter the name of your point");
  list->insert ("AKAs", "Enter your FTN addresses");
  list->insert ("Uplinks", "Enter your uplinks' addresses");
  list->insert ("Passwords", "Enter your passwords");
#ifdef MAILER_BINKP
  list->insert ("IP numbers", "Enter your uplinks IP numbers or hostnames");
#endif
  list->insert_data ((char *) cfg.user_name, 20, 1, 0);
  list->insert_data ((char *) cfg.location, 20, 2, 1);
  list->insert_data ((char *) cfg.point_name, 20, 3, 2);
  do
  {
    choice = list->play (choice);
    switch (choice)
    {
      case ABORT:
	done = TRUE;
	break;
      case 1:
	list->win->get_line (20, 1, cfg.user_name, 64, 1);
	break;
      case 2:
	list->win->get_line (20, 2, cfg.location, 64, 1);
	break;
      case 3:
	list->win->get_line (20, 3, cfg.point_name, 64, 1);
	break;
      case 4:
	configure_akas ();
	break;
      case 5:
	configure_uplinks ();
	break;
      case 6: 
	configure_passwords ();
	break;
      case 7:
	configure_phone_numbers ();
	break;
    }
  } while (!done);
  delete list;
}

/* Configure the directories to use for inbound, outbound, etc */

void CONFIG::configure_directories(void)
{
  LISTBOX *list;
  bool done = FALSE;
  char choice = 1;

  list = new LISTBOX (0, 0, 50, 0, "Directories");
  list->insert ("System", "The main directory");
  list->insert ("Message base", "Where your message base should be located");
  list->insert ("Inbound", "Where incoming files should be located");
  list->insert ("Outbound", "Where outgoing files should be located");
  list->insert ("Temp", "Where temporary files should be located");
  list->insert_data ((char *) cfg.system_path, 16, 1, 0);
  list->insert_data ((char *) cfg.msgbase_path, 16, 2, 1);
  list->insert_data ((char *) cfg.inbound_path, 16, 3, 2);
  list->insert_data ((char *) cfg.outbound_path, 16, 4, 3);
  list->insert_data ((char *) cfg.temp_path, 16, 5, 4);

  do
  {
    choice = list->play (choice);
    if (choice == ABORT) 
      done = TRUE;
    switch (choice)
    {
      case 1:
	list->win->get_line (16, 1, cfg.system_path, 80, 1);
	fix_path (cfg.system_path);
	list->win->put (16, 1, cfg.system_path, cfg.color[MISC_NORMAL]);
	break;
      case 2: 
	list->win->get_line (16, 2, cfg.msgbase_path, 80, 1);
	fix_path (cfg.msgbase_path);
	list->win->put (16, 2, cfg.msgbase_path, cfg.color[MISC_NORMAL]);
	break;
      case 3: 
	list->win->get_line (16, 3, cfg.inbound_path, 80, 1);
	fix_path (cfg.inbound_path);
	list->win->put (16, 3, cfg.inbound_path, cfg.color[MISC_NORMAL]);
	break;
      case 4:
	list->win->get_line (16, 4, cfg.outbound_path, 80, 1);
	fix_path (cfg.outbound_path);
	list->win->put (16, 4, cfg.outbound_path, cfg.color[MISC_NORMAL]);
	break;
      case 5: 
	list->win->get_line (16, 5, cfg.temp_path, 80, 1); 
	fix_path (cfg.temp_path);
	list->win->put (16, 5, cfg.temp_path,cfg.color[MISC_NORMAL]);
	break;
    }
  } while (!done);

  delete list;
}

/* Configure some other miscellaneous stuff */

void CONFIG::configure_misc (Window *win)
{
  LISTBOX *list;
  char choice = 1;
  bool done = FALSE;

  list = new LISTBOX (0, 0, 70, 0, "Miscellaneous");
  list->insert ("Quoting style", "Toggle quoting style (alternatives are >, xy> and XY>)");
  list->insert ("Mailer", "Select which mailer to call when Alt-P is pressed");
  list->insert ("Confirm on exit", "If Yes, ask for confirmation on program exit");
  list->insert ("Auto-jump to next area", "Jump to next area with new mail when pressing right arrow at the end of an area");
  list->insert ("Use separator line", "Show a separator line in the reader");
  list->insert ("Scan on exit", "Scan the message bases for outgoing mail on exit");
  list->insert_data (quote_type[cfg.quote_type], 35, 1);
  list->insert_data (cfg.poll_cmd, 35, 2);
  list->insert_data (yes_no[cfg.confirm_exit], 35, 3);
  list->insert_data (yes_no[cfg.jump_to_next], 35, 4);
  list->insert_data (yes_no[cfg.separating_line], 35, 5);
  list->insert_data (yes_no[cfg.scan_on_exit], 35, 6);

  do
  {
    choice = list->play (choice);
    switch (choice)
    {
      case 1: 
	cfg.quote_type++;
	if (cfg.quote_type == 3) cfg.quote_type = 0;
	list->win->put (35, 1, quote_type[cfg.quote_type], cfg.color[MISC_NORMAL]);
	break;
      case 2: 
	list->win->get_line (35, 2, cfg.poll_cmd, 80, 1);
	break;
      case 3:
	cfg.confirm_exit++;
	list->win->put (35, 3, yes_no[cfg.confirm_exit], cfg.color[MISC_NORMAL]);
	break;
      case 4:
	cfg.jump_to_next++;
	list->win->put (35, 4, yes_no[cfg.jump_to_next], cfg.color[MISC_NORMAL]);
	break;
      case 5:
	cfg.separating_line++;
	list->win->put (35, 5, yes_no[cfg.separating_line], cfg.color[MISC_NORMAL]);
	break;
      case 6:
	cfg.scan_on_exit++;
	list->win->put (35, 6, yes_no[cfg.scan_on_exit], cfg.color[MISC_NORMAL]);
	break;
      case ABORT: 
	done = TRUE;
	break;
    }
  } while (!done);
  delete list;
}

/* Configure the chosen area */

void CONFIG::configure_area (word which)
{
  LISTBOX *list;
  LISTBOX *list2;
  word c;
  sword a = 1;
  char choice = 1;
  bool done = FALSE;

  list = new LISTBOX (0, 0, 60, 0, "Configure Area");
  list->insert ("Description", "Description of area");
  list->insert ("Reply area", "Select in which area replies to messages in this area should be written");
  list->insert ("AKA", "Select what AKA to use for this area");
  list->insert ("Uplink", "Select what uplink to use in this area");
  list->insert ("Flags", "Configure miscellaneous settings regarding this area");
  list->insert_data (area[which]->comment, 20, 1, 0);
  list->insert_data (area[area[which]->reply_area]->comment, 20, 2, 0);
  list->insert_data (binToStrAddr (cfg.aka[area[which]->aka]), 20, 3, 0);
  list->insert_data (binToStrAddr (cfg.uplink[area[which]->uplink]), 20, 4, 0);
  
  do
  {
    choice = list->play (choice);
    switch (choice)
    {
      case ABORT: 
	done = TRUE;
	break;
      case 1: 
	list->win->get_line (20, 1, area[which]->comment, 79, 1);
	break;
      case 2:
	a = 1;
	 
	list2 = new LISTBOX ("Choose area");
	for (c = 1; c < area_header.areas; c++)
	{
	  list2->insert (area[c]->name, area[c]->comment);
	}
	a = list2->play (a);
	if (a != ABORT)
	  area[which]->reply_area = a;
	delete list2;
	list->win->put (20, 2, area[area[which]->reply_area]->comment, cfg.color[MISC_NORMAL]);
	break;
      case 3:
	a = area[which]->aka + 1;
	  
	list2 = new LISTBOX ("Choose AKA");
	for (c = 0; cfg.aka[c].zone; c++)
	  list2->insert (binToStrAddr (cfg.aka[c]), "");
	a = list2->play (a);
	if (a != ABORT)
	  area[which]->aka = a - 1;
	delete list2;
	break;
      case 4:
	a = area[which]->uplink + 1;
	  
	list2 = new LISTBOX ("Choose uplink");
	for (c = 0; cfg.uplink[c].zone; c++)
	  list2->insert (binToStrAddr (cfg.uplink[c]), "");
	a = list2->play (a);
	if (a != ABORT)
	  area[which]->uplink = a - 1;
	delete list2;
	break;
    }
  } while (!done);
  delete list;
}

/* Configure areas */

void CONFIG::configure_areas (void)
{
  LISTBOX *list;
  sword choice = 1;
  bool done = FALSE;

  list = new LISTBOX ("Choose area");
  for (dword c = 1; c < area_header.areas; c++)
    list->insert (area[c]->name, area[c]->comment);
  
  do
  {
    choice = list->play (choice);
    if (choice == ABORT) done = TRUE;
    else 
      configure_area (choice);
  } while (!done);
  write_areas ();
  delete list;
}

/* Configure the colors used by the program */

void CONFIG::configure_colors (void)
{
  LISTBOX *list;
  char choice = 1;
  bool done = false;

  list = new LISTBOX ("Color configuration");
  list->insert ("Choose color scheme", "Choose between predefined color schemes if you don't want to make your own");
  list->insert ("Configure reader", "Configure the colors used in the "
		"message reader");
  list->insert ("Configure editor", "Configure the colors used in the message editor");
  list->insert ("Configure miscellaneous stuff", "Configure other colors not affected by the above");

  do
  {
    choice = list->play (choice);
    switch (choice)
    {
      case 1: choose_color_scheme (); break;
      case 2: configure_reader_colors (); break;
      case 3: configure_editor_colors (); break;
      case 4: configure_misc_colors (); break;
      case ABORT: done = YES; break;
    }
      
  } while (!done);
  delete list;
}

/* Choose a color scheme */

void CONFIG::choose_color_scheme (void)
{
  LISTBOX *list;
  sword choice = 1;
  bool done = FALSE;

  list = new LISTBOX ("Choose scheme");
  for (uchar c = 0; c < SCHEMES; c++)
    list->insert (scheme_description[c], "You must restart tanstaafl for the changes to take full effect");
  
  do
  {
    choice = list->play (choice);
    if (choice == ABORT) done = TRUE;
    else 
    {
      memcpy (cfg.color, color_scheme[choice - 1], 64);
      ui.set_colors (cfg.color[MISC_WINDOW], cfg.color[MISC_TITLE],
		     cfg.color[MISC_NORMAL], cfg.color[MISC_CHOSEN],
		     cfg.color[MISC_STATUS]);
      done = TRUE;
    }
  } while (!done);
  delete list;
}

/* Configure the colors used by the message reader */

void CONFIG::configure_reader_colors (void)
{
  byte ch;
  bool done = FALSE;
  char tmpstr[256];
  dword c = 0, d = 0;
  uchar x = 0;
  struct tm *tblock;
  char msgtxt[] =
    "\001MSGID: 2:206/145.7 12345678\r\001REPLY: 2:206/145.2000\r\r pl> Har du \
kollat på den senaste versionen av tanstaafl?\r\rJo, den är verkligen extremt \
bra. Nu kan man tillochmed ställa in färgerna! Han som gjorde det programmet \
måste vara extremt cool.\r\r---\r * Origin: nonis (2:206/145.7)\r";
  word txt_size = strlen (msgtxt);
  word rows;
  time_t t;
  byte choice = 0;
  byte color;
  
  screen.save ();
  screen.clear ();

  convert_charset (msgtxt, 1, 0);

  do
  {
    rows = 0;
      
    /* Replace all carriage returns with zeroes, for easy parsing */
      
    for (d = 0; d < txt_size; d++)
      if (msgtxt[d] == 0x0D) 
      {
	msgtxt[d] = 0;
	rows++;
      }
	
    /* If a line is longer than the screen width, it must be split. Let's do that. */
	
    x = d = 0;
	
    while (d < txt_size)
    {
      x++; d++;
	    
      if (x > screen.x_max && msgtxt[d])
      {
	x = 0;
	while (msgtxt[d - x] != ' ' && x < screen.x_max)
	  x++;
		
	if (x == screen.x_max) /* we could not split the line, so we don't */
	  x = 0;
	else
	  msgtxt[d - x] = 0;
		
	rows++;
      }
      else
	if (!msgtxt[d]) /* end of a line */
	  x = 0;
    }
	
    d = 0;
	
    while (d < txt_size)
      d += strlen ((char *) msgtxt + d) + 1;
	
    if (rows > screen.y_max - 6) 
      rows -= screen.y_max - 6;
    else
      rows = 0;
      
    /* Print the message header */
      
    screen.put (1, 1, "Date : ", cfg.color[READER_HEADER]);
    screen.put (1, 2, "From : ", cfg.color[READER_HEADER]);
    screen.put (1, 3, "To   : ", cfg.color[READER_HEADER]);
    screen.put (1, 4, "Subj : ", cfg.color[READER_HEADER]);
      
    sprintf (tmpstr, "Jun Fri 04, 04:42");
    screen.put (8, 1, tmpstr, cfg.color[READER_HEADER]);
    screen.put (8, 2, "Henrik Hallin", cfg.color[READER_HEADER]);
    for (c = 8 + strlen ("Henrik Hallin"); c < 51; c++)
      screen.put (c, 2, ' ', cfg.color[READER_HEADER]);
    sprintf (tmpstr, "Per Lundberg");
    screen.put (8, 3, tmpstr, cfg.color[READER_PERSONAL]);
    for (c = 8 + strlen (tmpstr); c < 51; c++)
      screen.put (c, 3, ' ', cfg.color[READER_PERSONAL]);
    screen.put (8, 4, "Uggla!", cfg.color[READER_HEADER]);
    for (c = 8 + strlen ("Uggla!"); c < screen.x_max + 1; c++)
      screen.put (c, 4, ' ', cfg.color[READER_HEADER]);
      
    /* Always print the sender adress */
      
    sprintf (tmpstr, "2:206/145.7");
    screen.put (51, 2, tmpstr, cfg.color[READER_HEADER]);
    for (x = 51 + strlen (tmpstr); x < screen.x_max + 1; x++)
      screen.put (x, 2, 32, cfg.color[READER_HEADER]);
      
    /* If netmail, also print the receiver adress */
      
    sprintf (tmpstr, "2:206/145.2000");
    screen.put (51, 3, tmpstr, cfg.color[READER_HEADER]);
    for (x = 51 + strlen (tmpstr); x < screen.x_max + 1; x++)
      screen.put (x, 3, 32, cfg.color[READER_HEADER]);
      
    /* Show message attributes */
      
    strcpy (tmpstr, "pvt loc ");
      
    for (c = 25; c < screen.x_max - strlen (tmpstr) + 1; c++)
      screen.put (c, 1, ' ', cfg.color[READER_HEADER]);
      
    screen.put (screen.x_max - strlen (tmpstr) + 1, 1, tmpstr, cfg.color[READER_HEADER]);
      
    /* Indicate whether there are comments to this message or not */
      
    screen.put (screen.x_max - 3, 2, 30, cfg.color[READER_HEADER]); /* Up */
      
    /* Write a nice separating line */
      
    screen.color = cfg.color[READER_HEADER];
    screen.hline (5);
      
    c = d = 0;
      
    /* Show a status line */
      
    screen.put (1, screen.y_max, "Area: NETMAIL", cfg.color[MISC_STATUS]);
      
    time (&t);
    tblock = localtime (&t);
    sprintf (tmpstr, "³ %02d:%02d ", tblock->tm_hour, tblock->tm_min);
    screen.put (screen.x_max - 7, screen.y_max, tmpstr, cfg.color[MISC_STATUS]);
      
    sprintf (tmpstr, "100%%  Current: 42, high: 42 ");
    screen.put (screen.x_max - 7 - strlen (tmpstr), screen.y_max, tmpstr, cfg.color[MISC_STATUS]);
      
    for (c = 7 + strlen ("NETMAIL"); c < screen.x_max - strlen (tmpstr) - 7; c++)
      screen.put (c, screen.y_max, ' ', cfg.color[MISC_STATUS]);
      
    /* And finally, display the message */
      
    for (c = 0; c < screen.y_max - 6; c++)
    {
      if (d < txt_size)
      {
	strcpy (tmpstr, (char *) msgtxt + d);
	      
	d += strlen (tmpstr) + 1;
	if (is_kludge (tmpstr)) 
	  color = cfg.color[READER_KLUDGE];
	else if (is_origin (tmpstr))
	  color = cfg.color[READER_ORIGIN];
	else if (is_quote (tmpstr))
	  color = cfg.color[READER_QUOTE];
	else
	  color = cfg.color[READER_NORMAL];
	screen.put (1, 6 + c, tmpstr, color);
      }
      else
	strcpy (tmpstr, "");
      for (x = strlen (tmpstr) + 1; x < screen.x_max + 1; x++)
	screen.put (x, 6 + c, ' ', cfg.color[READER_NORMAL]);
	  
    }
    for (c = 1; c < (screen.x_max - strlen (color_description[choice])) / 2; c++)
      screen.put (c, screen.y_max / 2, ' ', cfg.color[MISC_NORMAL]);
    screen.put ((screen.x_max - strlen (color_description[choice])) / 2,
		screen.y_max / 2, color_description[choice], cfg.color[MISC_NORMAL]);
      
    for (c = (screen.x_max - strlen (color_description[choice])) / 2 + strlen (color_description[choice]); c <= screen.x_max; c++)
      screen.put (c, screen.y_max / 2, ' ', cfg.color[MISC_NORMAL]);

    screen.refresh ();

    ch = keyboard.get ();
    switch (ch)
    {
      case ' ': /* Space */
	choice++;
	if (color_description[choice] == NULL)
	  choice = 0;
	break;
      case 77: /* Right Arrow */
	c = cfg.color[choice] % 16;
	cfg.color[choice] -= c;
	c++;
	if (c == 16)
	  c = 0;
	cfg.color[choice] += c;
	break;
      case 75: /* Left Arrow */
	c = cfg.color[choice] % 16;
	cfg.color[choice] -= c;
	if (c)
	  c--;
	else
	  c = 15;
	cfg.color[choice] += c;
	break;
      case 72: /* Up Arrow */
	c = cfg.color[choice] / 16;
	cfg.color[choice] -= c * 16;
	if (c)
	  c--;
	else
	  c = 15;
	cfg.color[choice] += c * 16;
	break;
      case 80: /* Down Arrow */
	c = cfg.color[choice] / 16;
	cfg.color[choice] -= c * 16;
	c++;
	if (c == 16)
	  c = 0;
	cfg.color[choice] += c * 16;
	break;
      case 27: /* Escape */
	done = 1;
	break;
    }
  } while (!done);
  screen.restore ();
}

/* Configure miscellaneous colors */

void CONFIG::configure_misc_colors (void)
{
  byte ch;
  bool done = FALSE;
  byte choice = 0x20; /* misc colors start at 0x20 */
  byte c;

  screen.save ();
  screen.clear ();

  do
  {
    screen.clear (cfg.color[MISC_BACKGROUND] * 256 + 176);
    for (c = 0; c < screen.x_max; c++)
      screen.put (c + 1, screen.y_max, 32, cfg.color[MISC_STATUS]);
      
    screen.center (screen.y_max, string ("tanstaafl v") +
		   string (program_version) + string 
		   (" by Per Lundberg   Press F1 for Help"), 
		   cfg.color[MISC_STATUS]);
      
    screen.window (screen.x_max / 2 - 15, screen.y_max / 2 - 2,
		   screen.x_max /2 + 15, screen.y_max / 2 + 2, 
		   cfg.color[MISC_WINDOW], cfg.color[MISC_TITLE],
		   "Window title");

    screen.put (screen.x_max / 2 - 13, screen.y_max / 2 - 1, "Normal text", cfg.color[MISC_NORMAL]);
    screen.put (screen.x_max / 2 - 13, screen.y_max / 2, "Highlighted text", cfg.color[MISC_HIGHLIGHT]);
    screen.put (screen.x_max / 2 - 13, screen.y_max / 2 + 1, "Current choice", cfg.color[MISC_CHOSEN]);

    screen.put ((screen.x_max - strlen (color_description[choice])) / 2,
		screen.y_max / 2 + 5, color_description[choice], cfg.color[MISC_NORMAL]);
    screen.refresh ();

    ch = keyboard.get ();
    switch (ch)
    {
      case ' ': /* Space */
	choice++;
	if (color_description[choice] == NULL)
	  choice = 0x20; /* misc colors start at 0x20 */
	break;
      case 77: /* Right Arrow */
	c = cfg.color[choice] % 16;
	cfg.color[choice] -= c;
	c++;
	if (c == 16)
	  c = 0;
	cfg.color[choice] += c;
	break;
      case 75: /* Left Arrow */
	c = cfg.color[choice] % 16;
	cfg.color[choice] -= c;
	if (c)
	  c--;
	else
	  c = 15;
	cfg.color[choice] += c;
	break;
      case 72: /* Up Arrow */
	c = cfg.color[choice] / 16;
	cfg.color[choice] -= c * 16;
	if (c)
	  c--;
	else
	  c = 15;
	cfg.color[choice] += c * 16;
	break;
      case 80: /* Down Arrow */
	c = cfg.color[choice] / 16;
	cfg.color[choice] -= c * 16;
	c++;
	if (c == 16)
	  c = 0;
	cfg.color[choice] += c * 16;
	break;
      case 27: /* Escape */
	done = 1;
	break;
    }
  } while (!done);
  screen.restore ();
}

/* Configure the colors used by the editor */

void CONFIG::configure_editor_colors (void)
{
  byte ch;
  bool done = FALSE;
  byte choice = 0x10; /* editor colors start at 0x10 */
  byte c;
  char tmpstr[256];

  screen.save ();
  screen.clear ();

  do
  {
    screen.put (1, 1, " hh> tanstaafl verkar faktiskt bli ett *ruskigt* elajt program.\x0D", cfg.color[EDITOR_QUOTE]);
    screen.put (1, 3, "Jo, det „r extremt coolt.\x0D", cfg.color[EDITOR_NORMAL]);
    screen.put (1, 5, " * Origin: chaos (2:206/145.2000)\x0D", cfg.color[EDITOR_ORIGIN]);

    sprintf (tmpstr, " %s %s", program_name, program_version);
    screen.put (1, screen.y_max, tmpstr, cfg.color[MISC_STATUS]);
    c = strlen (tmpstr) + 1;
    sprintf (tmpstr, "³ %3u : %4u ", 42, 42);
    for (; c < screen.x_max - strlen (tmpstr); c++)
      screen.put (c, screen.y_max, ' ', cfg.color[MISC_STATUS]);
    screen.put (screen.x_max - strlen (tmpstr), screen.y_max, tmpstr,
		cfg.color[MISC_STATUS]);
      
    for (c = 1; c < (screen.x_max - strlen (color_description[choice])) / 2; c++)
      screen.put (c, screen.y_max / 2, ' ', cfg.color[MISC_NORMAL]);
    screen.put ((screen.x_max - strlen (color_description[choice])) / 2,
		screen.y_max / 2, color_description[choice], cfg.color[MISC_NORMAL]);

    for (c = (screen.x_max - strlen (color_description[choice])) / 2 + strlen (color_description[choice]); c <= screen.x_max; c++)
      screen.put (c, screen.y_max / 2, ' ', cfg.color[MISC_NORMAL]);
    screen.refresh ();

    ch = keyboard.get ();
    switch (ch)
    {
      case ' ': /* Space */
	choice++;
	if (color_description[choice] == NULL)
	  choice = 0x10; /* editor colors start at 0x10 */
	break;
      case 77: /* Right Arrow */
	c = cfg.color[choice] % 16;
	cfg.color[choice] -= c;
	c++;
	if (c == 16)
	  c = 0;
	cfg.color[choice] += c;
	break;
      case 75: /* Left Arrow */
	c = cfg.color[choice] % 16;
	cfg.color[choice] -= c;
	if (c)
	  c--;
	else
	  c = 15;
	cfg.color[choice] += c;
	break;
      case 72: /* Up Arrow */
	c = cfg.color[choice] / 16;
	cfg.color[choice] -= c * 16;
	if (c)
	  c--;
	else
	  c = 15;
	cfg.color[choice] += c * 16;
	break;
      case 80: /* Down Arrow */
	c = cfg.color[choice] / 16;
	cfg.color[choice] -= c * 16;
	c++;
	if (c == 16)
	  c = 0;
	cfg.color[choice] += c * 16;
	break;
      case 27: /* Escape */
	done = 1;
	break;
    }
  } while (!done);
  screen.restore ();
}

/* Main configuration part */

void CONFIG::play (void)
{
  LISTBOX *list;
  bool done = FALSE;
  char l = 1; 
  char oldl;
  configuration_type old_cfg;

  memcpy (&old_cfg, &cfg, sizeof (configuration_type));
  
  list = new LISTBOX ("Configuration");
  list->insert ("Site info", "General system configuration");
  list->insert ("Directories", "Specify where to put things");
  list->insert ("Area configuration", "Configure your message areas (origins, descriptions and so on)");
  //list->insert ("Group configuration", "Configure message groups");
  list->insert ("Colors", "Customize the colorization of tanstaafl");
  list->insert ("Miscellaneous", "Miscellaneous options that do not fit elsewhere");
  list->insert ((char *) NULL, (char *) NULL);
  list->insert ("Exit and save", "Exit and save the current configuration");
  list->insert_hbar (6);
  
  do
  {
    oldl = l;
    l = list->play (l);

    if (l == ABORT && (memcmp (&cfg, &old_cfg, sizeof (configuration_type))))
    {
      l = ui.yesno ("Are you sure?");
      if (l == YES) 
      {
	memcpy (&cfg, &old_cfg, sizeof(configuration_type));
	done = ABORT;
      }
      else
	l = oldl;
    }
    else if (l == ABORT)
      done = ABORT;
    else
      switch (l)
      {
	case 1: configure_site_info (); break;
	case 2: configure_directories (); break;
	case 3: configure_areas (); break;
	  //        case 4: configure_groups (); break;
	case 4: configure_colors (); break;
	case 5: configure_misc (list->win); break;
	case 7: done = TRUE; break;
      }
  } while (!done);
  write ();

  delete list;
}

/* Read the configuration file from disk */

void CONFIG::read (void)
{
  char tmp[128];
  FILE *file;

#ifdef __linux__
  sprintf (tmp, "%s/.tanstaafl/%s", getenv ("HOME"), TANSTACFG);
  file = fopen (tmp, "rb");
#else
  file = fopen (TANSTACFG, "rb");
#endif

  if (file == NULL)
  {
    cfg.quote_type = 1; /* xy> */
    getcwd (tmp, 127);

    if (tmp[strlen (tmp) - 1] != '/') strcat (tmp, "/");
#ifdef __linux__
    strcat (tmp, ".tanstaafl/");
#endif    
    strcpy (cfg.system_path, tmp);
    sprintf (cfg.msgbase_path, "%smsgbase", tmp);
    sprintf( cfg.inbound_path, "%sinbound", tmp);
    sprintf (cfg.outbound_path, "%soutbound", tmp);
#ifdef __MSDOS__
    sprintf (cfg.temp_path, "%stmp", tmp);
#else
    strcpy (cfg.temp_path, "/tmp");
#endif

    /* set default colors */

    memcpy (cfg.color, color_scheme[0], 64);
    ui.set_colors (cfg.color[MISC_WINDOW], cfg.color[MISC_TITLE],
		   cfg.color[MISC_NORMAL], cfg.color[MISC_CHOSEN],
		   cfg.color[MISC_STATUS]);
    
    area_header.areas = 3;
    
    area[0] = new area_type; /* Hidden private messages */
    area[1] = new area_type; /* Private mail */
    area[2] = new area_type; /* Bad messages */
    
    memset (area[0], 0, sizeof (area_type));
    memset (area[1], 0, sizeof (area_type));
    memset (area[2], 0, sizeof (area_type));
    
    strcpy (area[0]->name, "HIDDEN");
    strcpy (area[0]->comment, "Hidden private mail");
    
    strcpy (area[1]->name, "NETMAIL");
    strcpy (area[1]->comment, "Private mail");
    area[1]->reply_area = 1;
    
    strcpy (area[2]->name, "BADMAIL");
    strcpy (area[2]->comment, "Bad messages");
    area[2]->reply_area = 2;
    
    play ();
    mkdir (cfg.msgbase_path, 0700);
    mkdir (cfg.inbound_path, 0700);
    mkdir (cfg.outbound_path, 0700);
    mkdir (cfg.temp_path, 0700);
  }
  else
  {
    fread (&cfg, sizeof (configuration_type), 1, file);
    if (cfg.version == 0) /* version 0 configuration file */
    {
      /* set default colors */

      memcpy (cfg.color, color_scheme[0], 64);

      cfg.version = 0x0001; /* version 1.0 */
      write ();
    }
    ui.set_colors (cfg.color[MISC_WINDOW], cfg.color[MISC_TITLE],
		   cfg.color[MISC_NORMAL], cfg.color[MISC_CHOSEN],
		   cfg.color[MISC_STATUS]);
    fclose (file);
  }
}

/* Write the configuration file to disk */

void CONFIG::write (void)
{
  char tmpstr[256];
  FILE *file;

  strcpy (cfg.signature, "tanstacfg\032");
#ifdef __linux__
  sprintf (tmpstr, "%s/.tanstaafl", getenv ("HOME"));
  if (mkdir (tmpstr, 0700) == -1)
  {
    if (errno != EEXIST)
      ui.infobox ("Error", string ("Could not create directory ") + string (tmpstr));
  }
  strcat (tmpstr, "/");
  strcat (tmpstr, TANSTACFG);
#else
  strcpy (tmpstr, cfg.system_path);
  strcat (tmpstr, "/");
  strcat (tmpstr, TANSTACFG);
#endif
  file = fopen (tmpstr, "wb");
  if (file == NULL)
  {      
    ui.infobox ("Error", string ("Could not open ") + string (tmpstr) + string (" for writing"));
    return;
  }
  fwrite (&cfg, sizeof (configuration_type), 1, file);
  fclose (file);
}

/* Read the area configuration file from disk */

void CONFIG::read_areas (void)
{
  char tmpstr[256];
  FILE *file;

#ifdef __linux__
  sprintf (tmpstr, "%s/.tanstaafl/%s", getenv ("HOME"), TANSTAREAS);
#else
  strcpy (tmpstr, cfg.system_path);
  strcat (tmpstr, "/");
  strcat (tmpstr, TANSTAREAS);
#endif
  file = fopen (tmpstr, "rb");
  
  if (file == NULL) /* create a new area configuration file */
  {
    area_header.areas = 3;
     
    area[0] = new area_type; /* Hidden private messages */
    area[1] = new area_type; /* Private mail */
    area[2] = new area_type; /* Bad messages */

    memset (area[0], 0, sizeof (area_type));
    memset (area[1], 0, sizeof (area_type));
    memset (area[2], 0, sizeof (area_type));

    strcpy (area[0]->name, "HIDDEN");
    strcpy (area[0]->comment, "Hidden private mail");

    strcpy (area[1]->name, "NETMAIL");
    strcpy (area[1]->comment, "Private mail");
    area[1]->reply_area = 1;

    strcpy (area[2]->name, "BADMAIL");
    strcpy (area[2]->comment, "Bad messages");
    area[2]->reply_area = 2;
  }
  else
  {
    fread (&area_header, sizeof (areaheader_type), 1, file);

    for (word c = 0; c < area_header.areas; c++)
    {
      area[c] = new area_type;
      memset (area[c], 0, sizeof (area_type));
      fread (area[c], sizeof (area_type), 1, file);
      if (!area[c]->reply_area) area[c]->reply_area = c;
    }
  }
}

/* Write the area configuration file to disk */

void CONFIG::write_areas (void)
{
  char tmpstr[256];
  FILE *file;

#ifdef __linux__
  sprintf (tmpstr, "%s/.tanstaafl/", getenv ("HOME"));
  if (mkdir (tmpstr, 0700) != -1)
  {
    ui.infobox ("Error", string ("Could not create directory ") + string (tmpstr));
  }
  strcat (tmpstr, TANSTAREAS);
#else
  strcpy (tmpstr, cfg.system_path);
  strcat (tmpstr, "/");
  strcat (tmpstr, TANSTAREAS);
#endif
  file = fopen (tmpstr, "wb");
  if (file == NULL)
  {
    ui.infobox ("Error", string ("Could not open ") + string (tmpstr) +
		string (" for writing"));
    return;
  }
  fwrite (&area_header, sizeof (areaheader_type), 1, file);
  for (word c = 0; c < area_header.areas; c++)
    fwrite (area[c], sizeof (area_type), 1, file);
  fclose (file);
}

/* Add an area to the area configuration */

word CONFIG::add_area (area_type new_area)
{
  area[area_header.areas] = new area_type;
  area[area_header.areas]->reply_area = area_header.areas;
  memcpy (area[area_header.areas], &new_area, sizeof (area_type));
  area_header.areas++;
  write_areas ();
  return area_header.areas - 1;
}
