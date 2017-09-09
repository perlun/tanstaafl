/* mailer.cc - BinkP mailer. Not finished, but started. */
/* Copyright 1999 Per Lundberg */

#include "tanstaafl.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#define M_NUL  0                            /* Ignored by binkp (data optionally Logged) */
#define M_ADR  1
#define M_PWD  2
#define M_FILE 3
#define M_OK   4                            /* The password is ok (data ignored) */
#define M_EOB  5                            /* End-of-batch (data ignored) */
#define M_GOT  6                            /* File received */
#define M_ERR  7                            /* Misc errors */
#define M_BSY  8                            /* All AKAs are busy */
#define M_GET  9                            /* Get a file from offset */
#define M_SKIP 10                           /* Skip a file */
#define M_MAX  10

typedef struct
{
  byte len_hi: 7; /* length of data */
  byte flag: 1;   /* 0 = data, 1 = command */
  byte len_lo;    /* length of data */
} __attribute__ ((packed)) binkphdr_type;

void sighandler (int signal);


MAILER::MAILER (void)
{
}

MAILER::~MAILER (void)
{
}

void MAILER::play (byte flags)
{
  int sock;
  struct sockaddr_in addr;
  struct hostent *hostinfo;
  char tmpstr[256];
  binkphdr_type hdr;
  bool done = false;
  Window *window;
  byte *buf;
  word size;
  address_type address[256];
  dword n, c;
  char *host = config.cfg.phone[0];
  struct tm *tblock;
  time_t t;

  screen.save ();
  screen.clear ();

  screen.color = config.cfg.color[MISC_STATUS];
  for (c = 0; c < screen.x_max; c++)
    screen.put (c + 1, screen.y_max, ' ');
  
  screen.center (screen.y_max, string ("tanstaafl mailer ") +
		 string (program_version) + string 
		 (" by Per Lundberg   Press F1 for Help"), config.cfg.color[MISC_STATUS]);

  for (c = 0; c < screen.x_max; c++)
    screen.put (c + 1, 1, ' ');
  
  screen.center (1, config.cfg.point_name);

  window = new Window (1, 2, screen.x_max, screen.y_max - 2, config.cfg.color[MISC_WINDOW], config.cfg.color[MISC_TITLE], "Status");

  if ((sock = socket (PF_INET, SOCK_STREAM, 0)) == -1)
    {  
      perror ("socket");
      return;
    }

  screen.refresh ();

  hostinfo = gethostbyname (host);

  if (hostinfo == NULL)
    {
      // FIXME: parse h_errno
      ui.infobox ("Error", string ("Could not resolv hostname: ") + string (strerror (errno)));
      delete window;
      screen.restore ();
     return;
    }

  addr.sin_family = AF_INET;
  addr.sin_port = htons (24554);
  addr.sin_addr = *(struct in_addr *) hostinfo->h_addr;
  
  if ((connect (sock, (struct sockaddr *) &addr, sizeof (addr))) == -1)
    {
      ui.infobox ("Error", string ("Could not connect: ") + string (strerror (errno)));
      delete window;
      screen.restore ();
      return;
    }
  
  /* Send our information */

  hdr.flag = 1;
  sprintf (tmpstr, "%cSYS %s", M_NUL, config.cfg.point_name);
  size = strlen (config.cfg.point_name) + 5;
  hdr.len_hi = size >> 8;
  hdr.len_lo = size & 0xFF;
  write (sock, &hdr, 2);
  write (sock, tmpstr, size);

  sprintf (tmpstr, "%cZYZ %s", M_NUL, config.cfg.user_name);
  size = strlen (config.cfg.user_name) + 5;
  hdr.len_hi = size >> 8;
  hdr.len_lo = size & 0xFF;
  write (sock, &hdr, 2);
  write (sock, tmpstr, size);

  sprintf (tmpstr, "%cLOC %s", M_NUL, config.cfg.location);
  size = strlen (config.cfg.location) + 5;
  hdr.len_hi = size >> 8;
  hdr.len_lo = size & 0xFF;
  write (sock, &hdr, 2);
  write (sock, tmpstr, size);

  sprintf (tmpstr, "%cNDL TCP,BINKP", M_NUL);
  size = strlen   ( " NDL TCP BINKP");
  hdr.len_hi = size >> 8;
  hdr.len_lo = size & 0xFF;
  write (sock, &hdr, 2);
  write (sock, tmpstr, size);

  t = time (NULL);
  tblock = localtime (&t);
  
  sprintf (tmpstr, "%cTIME %04u/%02u/%02u %02u:%02u:%02u", M_NUL,
	   tblock->tm_year, tblock->tm_mon + 1, tblock->tm_mday,
	   tblock->tm_hour, tblock->tm_min, tblock->tm_sec);
  size = strlen   ( " TIME 1234/56/78 12:34:56");
  hdr.len_hi = size >> 8;
  hdr.len_lo = size & 0xFF;
  write (sock, &hdr, 2);
  write (sock, tmpstr, size);

  sprintf (tmpstr, "%cVER %s %s", M_NUL, program_name, program_version);
  size = strlen (program_name) + strlen (program_version) + 6;
  hdr.len_hi = size >> 8;
  hdr.len_lo = size & 0xFF;
  write (sock, &hdr, 2);
  write (sock, tmpstr, size);

  sprintf (tmpstr, "%c", M_ADR);
  for (n = 0; n < 10 && config.cfg.aka[n].zone; n++)
    {
      strcat (tmpstr, binToStrAddr (config.cfg.aka[n]));
      strcat (tmpstr, "@fidonet ");
    }

  size = strlen (tmpstr) - 1;
  hdr.len_hi = size >> 8;
  hdr.len_lo = size & 0xFF;
  write (sock, &hdr, 2);
  write (sock, tmpstr, size);
  
  while (!done)
    {
      if (read (sock, &hdr, 2) == 0)
        break;
      size = (hdr.len_hi << 8) + hdr.len_lo;
      buf = new byte[size];
      if (read (sock, buf, size) == 0)
        break;
      printf ("%d\n", buf[0]);
      if (hdr.flag == 1)
        switch (buf[0])
          {
          case M_NUL:
            memcpy (tmpstr, buf + 1, size - 1);
            tmpstr[size - 1] = 0;
            window->put (string ("Remote info: ") + tmpstr);
            screen.refresh ();
            break;
          case M_ADR:
            for (n = 1, c = 0; n < size; c++)
              {
                address[c] = strToBinAddr ((char *) buf + n);
                n += strlen (binToStrAddr (address[c]));
                while (!isspace (buf[n])) n++; /* skip the domain part */
                window->put (string ("Remote address: ") + binToStrAddr (address[c]));
              }
            memcpy (tmpstr, buf + 1, size - 1);
            tmpstr[size - 1] = 0;
            screen.refresh ();

            /* Send the session password */
            
            sprintf (tmpstr, "%c%s", M_PWD, "warez");
            size = strlen (tmpstr);
            hdr.len_hi = size >> 8;
            hdr.len_lo = size & 0xFF;
            write (sock, &hdr, 2);
            write (sock, tmpstr, size);
            
            //            done = true;
            break;
          case M_OK:
            window->put ("Password protected session");
            break;
          case M_ERR:
            window->put ("Error: Password mismatch");
            done = true;
            break;
          case M_EOB:
            window->put ("Session done.");
            done = true;
            break;
          default:
            break;
          }
      delete buf;
      //      done = true;
    }

  close (sock);

  keyboard.get ();

  delete window;
  screen.restore ();
}
