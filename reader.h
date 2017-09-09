/* reader.h - this is -*- C++ -*- */
/* Copyright 1998-1999 Per Lundberg */

#ifndef __READER_H__
#define __READER_H__

#include "editor.h"

enum { ORIGINAL, REPLY, REPLY_TO_RECIPIENT, FORWARD };

#ifdef __cplusplus

class READER
{
  MSGBASE msgbase;
  EDITOR editor;
  msghdr_type *msghdr;
  dword last_message, last_area, last_flags;
  dword curmsg;
  dword msglen;
  word choice, add;
  dword *total, *last, *unread;

  void refresh_screen (word choice, word add);
  void refresh_line (word choice, word c, word add);
  void show_line (dword choice, dword add);
  void show_message (dword message, word area_number, word start_line, 
		     byte flags);
  void write_message (msghdr_type msghdr, byte *oldmsg, word area, word type);
  void export_message (dword message, word area_number, byte flags);
  void help (void);
  void search (word area_number, byte flags);
  int search_in_msg (__search_t *srch, dword message);

 public:

  dword rows;

  READER (void);
  ~READER (void);
  dword list_messages (dword message, word area_number);
  void open_area (word area_number);
  word choose_area (void);
  void play (void);
};

#endif /* __cplusplus */
#endif /* __READER_H__ */
