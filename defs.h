/* defs.h - general definitions */
/* Copyright 1998-1999 Per Lundberg */

#ifndef __DEFS_H__
#define __DEFS_H__

/* Defines */

#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE !TRUE
#endif
#define MAX 0xFFFFFFFF
#define SUCCESS 0
#define YES 1
#define NO 0
#define ABORT -1

/* Typedefs */

typedef unsigned char uchar;
typedef signed char schar;
typedef unsigned char byte;
typedef unsigned long dword;
typedef unsigned short word;
typedef signed long sdword;
typedef signed short sword;

#ifndef __cplusplus
typedef signed char bool;
#endif

#ifdef __cplusplus
class CLASS
{
};
typedef void (CLASS::*FPTR)(...);
#else
typedef void (*FPTR)();
#endif

#endif /* __DEFS_H__ */
