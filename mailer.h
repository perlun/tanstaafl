/* mailer.h - definition of the MAILER class */
/* Copyright 1999 Per Lundberg */

#ifndef __MAILER_H__
#define __MAILER_H__

#ifdef __cplusplus
class MAILER
{
public:
  MAILER (void);
  ~MAILER (void);
  void play (byte flags);
};
#endif

#endif
