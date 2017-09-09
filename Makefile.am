## Process this file with automake to produce Makefile.in

##   This program is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License as published by
##   the Free Software Foundation; either version 2 of the License, or
##   (at your option) any later version.
##   This program is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.
##   You should have received a copy of the GNU General Public License
##   along with this program; if not, write to the Free Software
##   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

INCLUDES = -I.

bin_PROGRAMS = tanstaafl

tanstaafl_SOURCES = build_number.c compress.cc compress_auto.c \
configuration.cc crc32.cc editor.cc fido.c maintenance.cc misc.cc \
msgbase.cc reader.cc scanner.cc tanstaafl.cc tosser.cc \
build_number.h compress.h configuration.h crc32.h defs.h editor.h \
fido.h mailer.h maintenance.h misc.h msgbase.h reader.h scanner.h \
tanstaafl.h tosser.h $(IP) $(EXTERNAL) $(MODEM)

if IP
EXTRA_DEFS=-DMAILER_IP
IP = mailer_ip.cc
endif
if MODEM
EXTRA_DEFS=-DMAILER_MODEM
MODEM = mailer_modem.cc
endif
if EXTERNAL
EXTRA_DEFS=-DMAILER_EXTERNAL
EXTERNAL = mailer_external.cc
endif

EXTRA_tanstaafl_SOURCES = \
mailer_external.cc \
mailer_ip.cc \
mailer_internal.cc

EXTRA_DIST = \
build.awk \
compress.txt \
compress.pl

clean-local:
	rm *~

DEFS=$(EXTRA_DEFS) @DEFS@
CFLAGS=-g -O2 # -Wall -Werror
CXXFLAGS=-g -O2 # -Wall -Werror
LIBS=-lui

build_number.c:
	cat $(@) | $(AWK) -f build.awk > tmpfile
	mv tmpfile $(@)

compress_auto.c: compress.pl compress.txt
	perl compress.pl > $(@)
