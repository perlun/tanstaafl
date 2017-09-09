/* crc32.h */
/* This code was borrowed from the Snippets collection and was written by
   Gary S. Brown. You may use this as desired without restriction. */

#ifndef __CRC_H__
#define __CRC_H__

#include "defs.h"

#include <stdlib.h>           /* For size_t                 */

#define UPDC32(octet,crc) (crc_32_tab[((crc)\
     ^ ((byte)octet)) & 0xff] ^ ((crc) >> 8))

dword updateCRC32 (unsigned char ch, dword crc);
bool crc32file (char *name, dword *crc, long *charcnt);
dword crc32buf (char *buf, size_t len);

#endif /* __CRC_H__ */
