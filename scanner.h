/* scanner.h - definition of the scanner class. this is -*- C++ -*- */
/* Copyright 1998-1999 Per Lundberg */

#ifndef __SCANNER_H__
#define __SCANNER_H__

#ifdef __cplusplus

class SCANNER
{
  MSGBASE msgbase;

public:
  SCANNER (void);
  ~SCANNER (void);
  void play (byte flags);
};

#endif /* __cplusplus */

#endif /* __SCANNER_H__ */
