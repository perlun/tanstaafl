/* fido.c - miscellaneous FTN stuff */
/* Copyright 1998-1999 Per Lundberg */

/* This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include "fido.h"

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <ctype.h>

/* Convert a string to a binary 4D FidoNet-style address. Returns the
   resulting binary address. If the specified address is 5D, the domain is
   stripped. */

address_type strToBinAddr (char *address)
{
  address_type tmpaddr;
  int c;
  char slask[256];

  while (isspace (*address)) address++;
  memset (&tmpaddr, 0, sizeof (address_type));
  c = sscanf (address, "%hd:%hd/%hd.%hd@%s", &tmpaddr.zone, &tmpaddr.net, &tmpaddr.node,
	      &tmpaddr.point, slask);
  return tmpaddr;
}

char *binToStrAddr (address_type address)
{
  char *tmpstr = (char *) malloc (256); 

  memset (tmpstr, 0, 256);
  sprintf (tmpstr, "%u:%u/%u.%u", address.zone, address.net, address.node,
	   address.point);
  return tmpstr;
}
