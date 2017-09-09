/* mailer_internal.cc - internal mailer for serial modems */
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

  /* Put code in here.. */

  screen.restore ();
}
