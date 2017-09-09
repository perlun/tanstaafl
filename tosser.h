/* tosser.h - definition of the TOSSER class */
/* Copyright 1998-1999 Per Lundberg */

#ifndef __TOSSER_H__
#define __TOSSER_H__

#include "ui.h"
#include "window.h"

#ifdef __cplusplus
class TOSSER
{
  MSGBASE msgbase;
  sword toss (char *filename, Window *win);
 public:
  TOSSER (void);
  ~TOSSER (void);
  void play (byte flags);
};
#endif

#endif
