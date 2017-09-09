/* tanstaafl.cc - main module */  
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
#include "dialog.h"
#include "reader.h"
#include "build_number.h"

#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <sys/stat.h>
#include <getopt.h>
#include <string>
#include <fstream>

#ifdef __linux__
#include <sys/utsname.h>
#include <sys/mman.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <fcntl.h>
#elif __MSDOS__
#include <dpmi.h>
#endif

CONFIG config;

#ifdef __linux__
char *program_name = "tanstaafl/chaos";
#elif __MSDOS__
char *program_name = "tanstaafl/DOS";
#else
char *program_named ="tanstaafl/unknown";
#endif
char *program_version = "0.3.133.7";
char *copyright_string = "Copyright 1998-1999 Per Lundberg and others";
char *exit_message[7] = {
  " _______ _______ ______  _______ _______ _______ _______ _______ ___     ",
  "|       |   _   |   _  \\|   _   |       |   _   |   _   |   _   |   |    ",
  "|.|   | |.  1   |.  |   |   1___|.|   | |.  1   |.  1   |.  1___|.  |    ",
  "`-|.  |-|.  _   |.  |   |____   `-|.  |-|.  _   |.  _   |.  __) |.  |___ ",
  "  |:  | |:  |   |:  |   |:  1   | |:  | |:  |   |:  |   |:  |   |:  1   |",
  "  |::.| |::.|:. |::.|   |::.. . | |::.| |::.|:. |::.|:. |::.|   |::.. . |",
  "  `---' `--- ---`--- ---`-------' `---' `--- ---`--- ---`---'   `-------'"
};

byte color[] = { 14, 14, 10, 11, 3, 3, 3 };

uchar major = 0;
uchar minor = 2;
char **parameter;
word parameters;

typedef struct 
{
  char junk[8];
  short x;
  short y;
  char morejunk[112];
} __attribute__ ((packed)) pcx_header;

/* Introduction */

void TANSTAAFL::intro (void)
{
  Window *intro;
  char tmpstr[128];

#ifdef __linux
  struct utsname info;
#if 0  
  int fd;
  int pal[768];
  byte *bitmap;
  byte *fb = NULL;
  unsigned char c = 0, d, e;
  int f = 0;
  int l = 0;
  pcx_header header;
  word x, y;
  byte *p;
  fb_var_screeninfo fb_info;
  fd = open ("/dev/fb0", O_RDWR);
  if (fd != -1) /* we're running kernel 2.1/2.2 with LFB support */
  {
    ioctl (fd, FBIOGET_VSCREENINFO, &fb_info);
      
    fb = (byte *) mmap (fb, fb_info.xres * fb_info.yres, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if ((int) fb == -1)
      perror ("Could not mmap frame buffer");

    memcpy (&header, tanstaafl_image, 128);
    header.x++;
    header.y++;
    bitmap = new byte[header.x * header.y * 2];
    p = tanstaafl_image + 128;


    while (f != header.x * header.y)
    {
      c = p[l++];
	
      if ((c & 192) == 192)
      {
	e = p[l++];
	if (e != 255)
	  for (d = c ^ 192; d--; d)
	  {
	    bitmap[f++] = e;
	  }
      }
      else
	if (c != 255)
	  bitmap[f++] = c;
    }
    
    /*    for (f = 0; f < 256; f++)
	  {
	  pal[f * 3] = p[f * 3] >> 2;
	  pal[f * 3 + 1] = p[f * 3 + 1] >> 2;
	  pal[f * 3 + 2] = p[f * 3 + 2] >> 2;
	  }
    */

    l = 0;
    for (x = 0; x < header.x; x++)
      for (y = 0; y < header.y; y++)
      {
	fb[y * fb_info.xres + x] = 1; //bitmap[l++];
      };
    close (fd);
    delete bitmap;
  }
#endif
  uname (&info);
#elif __MSDOS__
  char vendor[128];
  __dpmi_regs regs;

  regs.x.ax = 0x3000;
  __dpmi_int (0x21, &regs);      /* Get DOS version */

  switch (regs.h.bh)
  {
  case 0: /* IBM, could be IBM DOS or OS/2 */
    if (regs.h.al == 20) /* OS/2 */
      sprintf (vendor, "OS/2 v%u.%u", regs.h.al / 10, regs.h.al % 10);
    else
      sprintf (vendor, "PC DOS %u.%u", regs.h.ah, regs.h.al);
    break;
  case 0xEF: /* Novell */
    sprintf (vendor, "Novell DOS %u.%u", regs.h.al, regs.h.ah); break;
  case 0xFF: /* Microsoft */
    sprintf (vendor, "MS-DOS %u.%u", regs.h.al, regs.h.ah); break;
  default: strcpy (vendor, "Unknown"); break;
  }
#endif
  
  sprintf (tmpstr, "%s version %s (build %d)", program_name, program_version, build_number);
  intro = new Window (0, 0, strlen (tmpstr) + 4, 7, config.cfg.color[MISC_WINDOW], config.cfg.color[MISC_TITLE],
		      program_name);
  intro->color = config.cfg.color[MISC_NORMAL];
  intro->center (1, tmpstr);
  intro->center (2, copyright_string);
  intro->hline (3, 0, intro->x_size - 1);
  string tmpstr2 = string ("Compiled at ") + string (__TIME__) +
    string (" on ") + string (__DATE__);
  intro->center (4, tmpstr2);
#ifdef __linux__
  intro->center (5, string ("Running under ") + string (info.sysname) +
		 string (" ") + string (info.release));
#else
  intro->center (5, string ("Running under ") + string (vendor)); 
#endif
  screen.refresh ();
  keyboard.get ();
  delete intro;
}

TANSTAAFL::TANSTAAFL (int argc, char **argv)
{
  config.color = false;
  
  parameters = argc;
  parameter = argv;

  struct option long_options[] =
  {
    { "color", no_argument, NULL, 'c' },
    { "help", no_argument, NULL, 'h' },
    { "poll", no_argument, NULL, 'p' },
    { "scan", no_argument, NULL, 's' },
    { "toss", no_argument, NULL, 't' },
    { 0, 0, 0, 0 }
  };
  int option_index;
  int d = 0;

  /* Let's parse the command line arguments */

  while (d != -1)
  {
    d = getopt_long (parameters, parameter, "chpts", long_options, &option_index);

    switch (d)
    {
    case -1:
      break;
    case 'h': /* help */
      screen.init ();
      keyboard.init ();
      show_help ();
      exit (1);
      break;
    case 'p': /* poll */
      config.read ();
      config.read_areas ();
      screen.init ();
      keyboard.init ();
      mailer.play (1);
      done ();
      exit (0);
      break;
    case 's': /* scan */
      config.read ();
      config.read_areas ();
      screen.init ();
      keyboard.init ();
      scanner.play (1);
      done ();
      exit (0);
      break;
    case 't': /* toss */
      config.read ();
      config.read_areas ();
      screen.init ();
      keyboard.init ();
      tosser.play (1);
      done ();
      exit (0);
      break;
    case 'c': /* color */
      config.color = true;
      break;
    default: /* unknown */
      show_help ();
      keyboard.done ();
      screen.done ();
      exit (0);
      break;
    }
  }

  /* Initialize screen and keyboard */

  screen.init ();
  keyboard.init ();

  /* Initialize some other stuff */

  init_misc ();

  /* Read the configuration */

  config.read ();
  config.read_areas ();

  /* Show some kind of "splash" window */
  
  intro ();

  /* Make sure the directories used by the program exists */

  mkdir (config.cfg.msgbase_path, 0700);
  mkdir (config.cfg.inbound_path, 0700);
  mkdir (config.cfg.outbound_path, 0700);
  mkdir (config.cfg.temp_path, 0700);
  chdir (config.cfg.system_path);
}

/* Deinitialization */

void TANSTAAFL::done (void)
{
  byte c;

  screen.clear ();
  screen.color = 7;
  for (c = 0; c < 7; c++)
    screen.center (c + 1, exit_message[c], color[c]);
#ifdef DEBIAN
  screen.center (8, string ("This program is free software. See ") +
		 string ("/usr/doc/copyright/GPL for more information."), 2);
#else
  screen.center (8, string ("This program is free software. See the file ") +
		 string ("COPYING for more information."), 2);
#endif
  screen.refresh ();
#ifdef __MSDOS__
  screen.gotoxy (1, 9);
#else
  screen.gotoxy (1, 10);
#endif
}

TANSTAAFL::~TANSTAAFL (void)
{
  done ();
}

void TANSTAAFL::show_help (void)
{
  puts ("\nUsage: tanstaafl [-h | --help ] [-p | --poll] [-s | --scan] [-t | --toss]");
  puts ("Copyright 1998-1999 Per Lundberg, Bartek Kania, Johannes Lundberg and others\n");
}

void TANSTAAFL::play (void)
{
  uchar c;
  bool done = FALSE;

  
  screen.clear (config.cfg.color[MISC_BACKGROUND] * 256 + 176);
  
  for (c = 0; c < screen.x_max; c++)
    screen.put (c + 1, screen.y_max, 32, config.cfg.color[MISC_STATUS]);

  screen.center (screen.y_max, string ("tanstaafl v") +
		 string (program_version) + string 
		 (" by Per Lundberg   Press F1 for Help"), config.cfg.color[MISC_STATUS]);
  screen.refresh ();

  do
  {
    c = keyboard.get ();
    switch (c)
    {
    case 0:
      c = keyboard.get ();
      switch (c)
      {   
        case 30: /* Alt-A, do it all */
          scanner.play (0);
          mailer.play (0);
          tosser.play (0);
          break;
        case 19: /* Alt-R, read mail */
          reader.play ();
          break;
        case 20: /* Alt-T, toss incoming mail */
          tosser.play (0);
          break;
        case 25: /* Alt-P, poll an uplink */
          mailer.play (0);
          break;
        case 31: /* Alt-S, scan out outgoing mail */
          scanner.play (0);
          break;
        case 45: /* Alt-X, exit tanstaafl */
          if (!config.cfg.confirm_exit)
            done = TRUE;
          else
            if (ui.yesno ("Do you really want to exit?") == YES)
              done = TRUE;
          break;
        case 46: /* Alt-C, configuration */
          config.play ();
          break;
        case 50: /* Alt-M, maintenance */
          maint.play ();
          break;
        case 59: /* F1, help screen */
          help ();
          break;
        default:
          break;
      }
      default:  
        break;
    }
  } while (!done);
  if (config.cfg.scan_on_exit)
    scanner.play (0);
}

void TANSTAAFL::help (void)
{
  Window *help;
  char c = 0;
  
  help = new Window (0, 0, 50, 14, config.cfg.color[MISC_WINDOW], config.cfg.color[MISC_TITLE], "Help");
  help->color = config.cfg.color[MISC_NORMAL];
  help->center (1, string (program_name) + string (" ") + string (program_version), config.cfg.color[MISC_HIGHLIGHT]);
  help->center (2, copyright_string);
  help->put (2, 4, "Alt-C  Configuration");
  help->put (2, 5, "Alt-M  Run message base maintenance");
  help->put (2, 6, "Alt-P  Poll uplink");
  help->put (2, 7, "Alt-R  Read mail");
  help->put (2, 8, "Alt-S  Scan out outgoing mail");
  help->put (2, 9, "Alt-T  Toss inbound mail");
  help->put (2, 10, string ("Alt-X  Exit ") + string (program_name));
  help->hline (11);
  help->center (12, "This program uses code from InfoZIP:s UnZip");

  screen.refresh ();
  do 
  {
    c = keyboard.get ();
  } while (c != 13 && c != 27);
  delete help;  
}

/* This is a good example of how a main function should look :) */

int main (int argc, char **argv)
{
  TANSTAAFL *tanstaafl;

  tanstaafl = new TANSTAAFL (argc, argv);
   
  tanstaafl->play ();

  delete tanstaafl;

}
