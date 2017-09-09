/* fido.h */
/* Copyright 1998-1999 Per Lundberg */

#ifndef __FIDO_H__
#define __FIDO_H__

#include "defs.h"

/* a type 2+ packet header */

#ifdef __cplusplus
extern "C" {
#endif
typedef struct
{
  word origNode;
  word destNode;
  word year;
  word month;
  word day;
  word hour;
  word min;
  word sec;
  word baud;				               /* unused */
  word packetVersion;		                     /* always 2 */
  word origNet;
  word destNet;
  byte productCodeLo;	             /* low part of product code */
  byte productRevMaj;	       /* major part of product revision */
  byte password[8];
  word qOrigZone;		     /* used by ZMailQ and QMail */
  word qDestZone;		     /* used by ZMailQ and QMail */
  word filler;	                        	       /* unused */
  word CWValidation;   /* byte-swapped capability word validation */
  byte productCodeHi;	            /* high part of product code */
  byte productRevMin;	       /* minor part of product revision */
  word capabilityWord;
  word origZone;
  word destZone;
  word origPoint;
  word destPoint;
  dword productData;	    /* may be used for anything we want */
} __attribute__ ((packed)) pkthdr_type;

typedef struct
{
  word zone;
  word net;
  word node;
  word point;
} __attribute__ ((packed)) address_type;

/* a message header. comes directly after the packet header. */

typedef struct
{
  word id;		  		       	                 /* always 2 */
  word orig_node;                 /* the system where the packet was created */
  word dest_node;                  /* the system where the packet is heading */
  word orig_net;
  word dest_net;
  word attribute;
  word cost;
  char date[20];                      /* in the format "09 Nov 98  22:09:10" */
} __attribute__ ((packed)) packed_msg_type;

address_type strToBinAddr (char *address);
char *binToStrAddr (address_type address);

#ifdef __cplusplus
};
#endif
#endif /* __FIDO_H__ */
