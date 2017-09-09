/* misc.cc - miscellaneous routines that doesn't fit elsewhere */
/* Copyright 1998-1999 Per Lundberg */
/* Copyright 1999 Bartek Kania */

#include "tanstaafl.h"

#ifdef __MSDOS__
#include <sys/types.h>
#endif
#include <string.h>
#include <regex.h>
#include <locale.h>
#include <ctype.h>

char *weekday[7] =
{
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

char *month[12] =
{
  "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov",
  "Dec"
};

const byte charmap[2][2][256]=
{
  {
    { /* Linux latin1 -- NO CONVERSION */
      0,   1,   2,   3,   4,   5,   6,   7,
      8,   9,  10,  11,  12,  13,  14,  15,
      16,  17,  18,  19,  20,  21,  22,  23,
      24,  25,  26,  27,  28,  29,  30,  31,
      32,  33,  34,  35,  36,  37,  38,  39,
      40,  41,  42,  43,  44,  45,  46,  47,
      48,  49,  50,  51,  52,  53,  54,  55,
      56,  57,  58,  59,  60,  61,  62,  63,
      64,  65,  66,  67,  68,  69,  70,  71,
      72,  73,  74,  75,  76,  77,  78,  79,
      80,  81,  82,  83,  84,  85,  86,  87,
      88,  89,  90,  91,  92,  93,  94,  95,
      96,  97,  98,  99, 100, 101, 102, 103,
      104, 105, 106, 107, 108, 109, 110, 111,
      112, 113, 114, 115, 116, 117, 118, 119,
      120, 121, 122, 123, 124, 125, 126, 127,
      128, 129, 130, 131, 132, 133, 134, 135,
      136, 137, 138, 139, 140, 141, 142, 143,
      144, 145, 146, 147, 148, 149, 150, 151,
      152, 153, 154, 155, 156, 157, 158, 159,
      160, 161, 162, 163, 164, 165, 166, 167,
      168, 169, 170, 171, 172, 173, 174, 175,
      176, 177, 178, 179, 180, 181, 182, 183,
      184, 185, 186, 187, 188, 189, 190, 191,
      192, 193, 194, 195, 196, 197, 198, 199,
      200, 201, 202, 203, 204, 205, 206, 207,
      208, 209, 210, 211, 212, 213, 214, 215,
      216, 217, 218, 219, 220, 221, 222, 223,
      224, 225, 226, 227, 228, 229, 230, 231,
      232, 233, 234, 235, 236, 237, 238, 239,
      240, 241, 242, 243, 244, 245, 246, 247,
      248, 249, 250, 251, 252, 253, 254, 255,
    },
    { /* Conversion table from `latin1' charset to `ibmpc' charset. */
      /* Each input char transforms into an output string. */
      0,   1,   2,   3,   4,   5,   6,   7,     /*   0 -   7 */
      8,   9,  10,  11,  12,  13,  14,  15,     /*   8 -  15 */
      16,  17,  18,  19, 182, 186,  22,  23,     /*  16 -  23 */
      24,  25,  26,  27,  28,  29,  30,  31,     /*  24 -  31 */
      32,  33,  34,  35,  36,  37,  38,  39,     /*  32 -  39 */
      40,  41,  42,  43,  44,  45,  46,  47,     /*  40 -  47 */
      48,  49,  50,  51,  52,  53,  54,  55,     /*  48 -  55 */
      56,  57,  58,  59,  60,  61,  62,  63,     /*  56 -  63 */
      64,  65,  66,  67,  68,  69,  70,  71,     /*  64 -  71 */
      72,  73,  74,  75,  76,  77,  78,  79,     /*  72 -  79 */
      80,  81,  82,  83,  84,  85,  86,  87,     /*  80 -  87 */
      88,  89,  90,  91,  92,  93,  94,  95,     /*  88 -  95 */
      96,  97,  98,  99, 100, 101, 102, 103,     /*  96 - 103 */
      104, 105, 106, 107, 108, 109, 110, 111,     /* 104 - 111 */
      112, 113, 114, 115, 116, 117, 118, 119,     /* 112 - 119 */
      120, 121, 122, 123, 124, 125, 126, 127,     /* 120 - 127 */
      199, 252, 233, 226, 228, 224, 229, 231,     /* 128 - 135 */
      234, 235, 232, 239, 238, 236, 196, 197,     /* 136 - 143 */
      201, 181, 198, 244, 247, 242, 251, 249,     /* 144 - 151 */
      223, 214, 220, 243, 183, 209, 158, 159,     /* 152 - 159 */
      255, 173, 155, 156, 177, 157, 188,  21,     /* 160 - 167 */
      191, 169, 166, 174, 170, 237, 189, 187,     /* 168 - 175 */
      248, 241, 253, 179, 180, 230,  20, 250,     /* 176 - 183 */
      184, 185, 167, 175, 172, 171, 190, 168,     /* 184 - 191 */
      192, 193, 194, 195, 142, 143, 146, 128,     /* 192 - 199 */
      200, 144, 202, 203, 204, 205, 206, 207,     /* 200 - 207 */
      208, 165, 210, 211, 212, 213, 153, 215,     /* 208 - 215 */
      216, 217, 218, 219, 154, 221, 222, 225,     /* 216 - 223 */
      133, 160, 131, 227, 132, 134, 145, 135,     /* 224 - 231 */
      138, 130, 136, 137, 141, 161, 140, 139,     /* 232 - 239 */
      240, 164, 149, 162, 147, 245, 148, 246,     /* 240 - 247 */
      176, 151, 163, 150, 129, 178, 254, 152,     /* 248 - 255 */
    }
  },
  {
    { /* Conversion table from `ibmpc' charset to `latin1' charset. */
      0,   1,   2,   3,   4,   5,   6,   7,     /*   0 -   7 */
      8,   9,  10,  11,  12,  13,  14,  15,     /*   8 -  15 */
      16,  17,  18,  19, 182, 167,  22,  23,     /*  16 -  23 */
      24,  25,  26,  27,  28,  29,  30,  31,     /*  24 -  31 */
      32,  33,  34,  35,  36,  37,  38,  39,     /*  32 -  39 */
      40,  41,  42,  43,  44,  45,  46,  47,     /*  40 -  47 */
      48,  49,  50,  51,  52,  53,  54,  55,     /*  48 -  55 */
      56,  57,  58,  59,  60,  61,  62,  63,     /*  56 -  63 */
      64,  65,  66,  67,  68,  69,  70,  71,     /*  64 -  71 */
      72,  73,  74,  75,  76,  77,  78,  79,     /*  72 -  79 */
      80,  81,  82,  83,  84,  85,  86,  87,     /*  80 -  87 */
      88,  89,  90,  91,  92,  93,  94,  95,     /*  88 -  95 */
      96,  97,  98,  99, 100, 101, 102, 103,     /*  96 - 103 */
      104, 105, 106, 107, 108, 109, 110, 111,     /* 104 - 111 */
      112, 113, 114, 115, 116, 117, 118, 119,     /* 112 - 119 */
      120, 121, 122, 123, 124, 125, 126, 127,     /* 120 - 127 */
      199, 252, 233, 226, 228, 224, 229, 231,     /* 128 - 135 */
      234, 235, 232, 239, 238, 236, 196, 197,     /* 136 - 143 */
      201, 230, 198, 244, 246, 242, 251, 249,     /* 144 - 151 */
      255, 214, 220, 162, 163, 165, 158, 159,     /* 152 - 159 */
      225, 237, 243, 250, 241, 209, 170, 186,     /* 160 - 167 */
      191, 169, 172, 189, 188, 161, 171, 187,     /* 168 - 175 */
      
      35,  35,  35, 124,  43, 124,  43,  46,
      46, 124, 124,  46,  39,  39,  39,  46,
      96,  43,  43,  43,  45,  43, 124,  43,
      96,  46,  61,  61, 124,  61,  61,  61,
      43,  61,  43,  96,  96,  46,  46,  43,
      61,  39,  46,  35,  35,  35,  35,  35,
    
      133, 223, 131, 227, 132, 134, 181, 135,     /* 224 - 231 */
      138, 130, 136, 137, 141, 173, 140, 139,     /* 232 - 239 */
      240, 177, 149, 155, 147, 245, 247, 148,     /* 240 - 247 */
      176, 151, 183, 150, 129, 178, 254, 160,     /* 248 - 255 */
    },
    { /* Linux latin1 -- NO CONVERSION */
      0,   1,   2,   3,   4,   5,   6,   7,
      8,   9,  10,  11,  12,  13,  14,  15,
      16,  17,  18,  19,  20,  21,  22,  23,
      24,  25,  26,  27,  28,  29,  30,  31,
      32,  33,  34,  35,  36,  37,  38,  39,
      40,  41,  42,  43,  44,  45,  46,  47,
      48,  49,  50,  51,  52,  53,  54,  55,
      56,  57,  58,  59,  60,  61,  62,  63,
      64,  65,  66,  67,  68,  69,  70,  71,
      72,  73,  74,  75,  76,  77,  78,  79,
      80,  81,  82,  83,  84,  85,  86,  87,
      88,  89,  90,  91,  92,  93,  94,  95,
      96,  97,  98,  99, 100, 101, 102, 103,
      104, 105, 106, 107, 108, 109, 110, 111,
      112, 113, 114, 115, 116, 117, 118, 119,
      120, 121, 122, 123, 124, 125, 126, 127,
      128, 129, 130, 131, 132, 133, 134, 135,
      136, 137, 138, 139, 140, 141, 142, 143,
      144, 145, 146, 147, 148, 149, 150, 151,
      152, 153, 154, 155, 156, 157, 158, 159,
      160, 161, 162, 163, 164, 165, 166, 167,
      168, 169, 170, 171, 172, 173, 174, 175,
      176, 177, 178, 179, 180, 181, 182, 183,
      184, 185, 186, 187, 188, 189, 190, 191,
      192, 193, 194, 195, 196, 197, 198, 199,
      200, 201, 202, 203, 204, 205, 206, 207,
      208, 209, 210, 211, 212, 213, 214, 215,
      216, 217, 218, 219, 220, 221, 222, 223,
      224, 225, 226, 227, 228, 229, 230, 231,
      232, 233, 234, 235, 236, 237, 238, 239,
      240, 241, 242, 243, 244, 245, 246, 247,
      248, 249, 250, 251, 252, 253, 254, 255,
    }
  }
};

char tmpstr[256];
regex_t reply;

/* Convert charsets */

void convert_charset (uchar *s, uchar from, uchar to)
{
  while (*s) 
    *s = charmap[from][to][*(s++)];
}

void convert_charset (char *s, uchar from, uchar to)
{
  while (*s) 
    *s = charmap[from][to][*(s++)];
}

/* Read a string from the specified file. When a NUL is reached, return */

void fgetsz (FILE *file, char *s, dword l)
{
  dword c = 0;
  uchar ch;

  do
  {
    ch = fgetc (file);
    if (!feof (file)) s[c++] = ch;
  } while (!feof (file) && ch && c != l);
  s[c] = 0;
}

void fix_path (char *s)
{
  while (*s)
  {
    if (*s == '\\') *s = '/';
    s++;
  }
}

/* Find a substring in a string, and return the rest of the line */

char *get_string (const char *string, const char *substring)
{
  char *p;
  word c = 0;

  p = strstr (string, substring);
  if (p == NULL)
  {
    tmpstr[0] = 0;
    return tmpstr;
  }
  p += strlen (substring);
  while (isspace (*p))
    p++;
  while (p[c] != 13 && p[c]) 
    c++;
  strncpy (tmpstr, p, c);
  tmpstr[c] = 0;
  return tmpstr;
}

/* If the string is a reply, return the initial letters */

char *get_quote (char *s)
{
  regmatch_t match;

  if (regexec (&reply, s, 1, &match, 0)) /* didnt match */
    return NULL;
  if (match.rm_so) return NULL;
  strncpy (tmpstr, s + match.rm_so, match.rm_eo);
  tmpstr[match.rm_eo] = 0;
  return tmpstr;
}

/* Determine whether a line should be treated as quoted */
/* FIXME: create a better algorithm for this.. */

bool is_quote (char *s)
{
  char tmpstr[10];
  memset (tmpstr, 0, 10);
  memcpy (tmpstr, s, 9);
  if (strchr (tmpstr, '>'))
    return TRUE;
  return FALSE;
}

/* Determine whether a line should be treated as kludge text */

bool is_kludge (char *s)
{
  if (s[0] == 1) return TRUE;
  get_string (s, "SEEN-BY: ");
  if (tmpstr[0]) return TRUE;
  return FALSE;
}

/* Determine whether a line should should be treated as origin text or not */

bool is_origin (char *s)
{
  if (!strncmp (s, "---", 3) || !strncmp (s,  " * Origin:", 10))
    return TRUE;
  return FALSE;
}


/* Convert a string to uppercase */

char *to_upper (char *s)
{
  char *p = s;

  while (*s)
  {
    switch (*s)
    {
      case 'å': *s = 'Å'; break;
      case 'ä': *s = 'Ä'; break;
      case 'ö': *s = 'Ö'; break;
      default:  *s = toupper (*s);
    }
    s++;
  }
  return p;
}

/* Convert a string to lowercase */

char *to_lower (char *s)
{
  char *p = s;

  while (*s)
  {
    switch (*s)
    {
      case 'Å': *s = 'å'; break;
      case 'Ä': *s = 'ä'; break;
      case 'Ö': *s = 'ö'; break;
      default:  *s = tolower (*s);
    }
    s++;
  } 
  return p;
}

/* Search using finite automata */

void tns_search_destroy (search_t sr)
{
  delete [] sr->func;
  delete sr;
}

search_t tns_search_init (byte *s)
{
  char *tmp, *ft;
  int i, j, k, s_len, st = s_len = strlen ((char *) s);
  search_t sr;

  if (!(sr = new __search_t)) return 0;
  if (!(sr->func = new char[st * 256]))
  {
    delete sr;
    return 0;
  }
  ft = sr->func;
  sr->end = s_len;

  if (!(tmp = new char[st + 1]))
  {
    tns_search_destroy (sr);
    return 0;
  }
  memset (tmp, 0, st + 1);

  for (j = 0; j < s_len; j++)
  {
    for (i = 0; i < 256; i++)
    {
      tmp[j] = i;
      k = j + 1;
      while (strncasecmp (&tmp[j - k + 1], (char *) s, k) && k) k--;
      *ft = k;
      ft++;
    }
    tmp[j] = s[j];
  }

  delete [] tmp;
  return sr;
}

int tns_search (__search_t *sr, uchar *m, int len)
{
  int st = 0;
  int i;

  for (i = 0; i < len; i++)
  {
    st = *(sr->func + (st << 8) + m[i]);
    if (st == sr->end) return i + 1;
  }
  return 0;
}


void init_misc (void)
{
  setlocale (LC_ALL, "");
#ifdef __linux__
  regcomp (&reply, "([[:space:]]*([A-Öa-ö]*)>+)+", REG_EXTENDED);
#else /* __MSDOS__ */
  regcomp (&reply, ":space:*[A-Za-z]*>*", 0);
#endif
}

/* Convert a string to stars, suitable for password printing */

char *string_to_stars (char *s)
{
  dword c;
  strcpy (tmpstr, s);
  
  for (c = 0; tmpstr[c]; c++)
  {
    tmpstr[c] = '*';
    s++;
  }
  return tmpstr;
}
