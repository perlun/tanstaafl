/* misc.h */
/* Copyright 1998-1999 Per Lundberg */

#ifndef __MISC_H__
#define __MISC_H__

#include <stdio.h>

typedef struct __search_t
{
  int end;
  char *func;
} *search_t;

extern char *month[12];
extern char *weekday[7];
extern const byte charmap[2][2][256];
void convert_charset (uchar *s, uchar from, uchar to);
void convert_charset (char *s, uchar from, uchar to);
void fix_path (char *s);
void fgetsz (FILE *file, char *s, dword l);
char *get_string (const char *string, const char *substring);
char *get_quote (char *s);
bool is_quote (char *s);
bool is_kludge (char *s);
bool is_origin (char *s);
char *to_lower (char *s);
char *to_upper (char *s);
int tns_search (__search_t *sr, byte *m, int len);
void tns_search_destroy (search_t sr);
void init_misc (void);
search_t tns_search_init (byte *s);
char *string_to_stars (char *string);
  
#endif /* !__MISC_H__ */
