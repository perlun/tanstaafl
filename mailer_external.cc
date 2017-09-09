/* mailer.cc - calls an external mailer */
/* Copyright 1999 Per Lundberg */

#include "tanstaafl.h"

MAILER::MAILER (void)
{
}

MAILER::~MAILER (void)
{
}

void MAILER::play (byte flags)
{
  screen.save ();
  screen.clear ();
  screen.show_cursor ();
  
  keyboard.done ();
  if (system ((char *) config.cfg.poll_cmd) == -1)
  {
    perror ("Could not execute mailer");
    keyboard.init ();
    ui.infobox ("Mailer", "Could not execute external mailer.");
  }

#if 0
  if (flags ^ 1)
  {
    keyboard.init ();
    keyboard.get ();
  }
  else
#endif

  keyboard.init ();

  screen.hide_cursor ();
  screen.restore ();
}
