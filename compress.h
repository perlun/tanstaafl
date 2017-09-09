/* compress.h - compression support */
/* copyright 1999 Per Lundberg */

#ifndef __COMPRESS_H__
#define __COMPRESS_H__

#include <defs.h>

typedef struct
{
  byte *id;
  byte *name;
} compress_type;

extern compress_type compress[];

extern bool uncompress (byte *filename);

#endif /* !__COMPRESS_H__ */
