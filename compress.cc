/* compress.c - compression support */
/* copyright 1999 Per Lundberg */

#include <compress.h>
#include <defs.h>
#include <keyboard_linux.h>
#include <ui.h>

#include <stdio.h>

/* Uncompress the given filename. If it is not detected as a valid archive,
   an error message is printed and ERROR is returned. */

bool uncompress (byte *filename)
{
  FILE *file;
  byte buffer[16];
  byte tmpstr[256];
  dword count;

  file = fopen ((char *) filename, "rb");
  if (!file)
    return ABORT;
  fread (buffer, 16, 1, file);
  fclose (file);

  for (count = 0; compress[count].id; count++)
    if (!strncmp ((char *) compress[count].id, (char *) buffer,
		  strlen ((char *) compress[count].id)))
    {
      sprintf ((char *) tmpstr, "%s %s", (char *) compress[count].name, (char *) filename);
      return system ((char *) tmpstr);
    }

  ui.infobox (string ("No valid uncompressor found for archive ") + string ((char *) filename));

  return ABORT;
}
