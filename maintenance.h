/* maintenance.h - definition of the maintenance class. this is -*- C++ -*- */
/* Copyright 1998-1999 Per Lundberg */

#ifndef __MAINTENANCE_H__
#define __MAINTENANCE_H__

#ifdef __cplusplus
class MAINTENANCE
{
  MSGBASE msgbase;

public:
  MAINTENANCE (void);
  ~MAINTENANCE (void);
  void play (void);
};
#endif /* __cplusplus */

#endif /* __MAINTENANCE_H__ */
