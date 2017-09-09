/* maintenance.cc - routines for message base maintenance */
/* Copyright 1998-1999 Per Lundberg */

#include "tanstaafl.h"
#include "maintenance.h"

MAINTENANCE::MAINTENANCE (void)
{
}

MAINTENANCE::~MAINTENANCE (void)
{
}

void MAINTENANCE::play (void)
{
  Window *win;

  win = new Window (screen.x_max / 2 - 25, screen.y_max / 2 - 3, 50, 5, 3, 11, "Maintenance");

  msgbase.init (config.cfg.msgbase_path);
  // msgbase.rebuild_msgidx ();  /* rebuild msgidx.tfl from msghdr.tfl */
  win->put ("Rebuilding msgstat.tfl");
  msgbase.rebuild_msgstat ();   /* rebuild msgstat.tfl from msghdr.tfl */
  win->put ("Rebuilding reply chains");
  keyboard.done ();
  msgbase.link (0, msgbase.get_number_of_messages ()); /* relink the message base */
  keyboard.init ();
  win->put ("Done. Press Enter");
  keyboard.get ();
  delete win;
}

