/* editor.h - this is -*- C++ -*- */
/* Copyright 1998-1999 Per Lundberg */

#ifndef __EDITOR_H__
#define __EDITOR_H__
#ifdef __cplusplus

class EDITOR
{
  char *msgbuf; 

  void EDITOR::show_statusline (byte x, word y, msghdr_type *msghdr);
  byte read_string (uchar x, uchar y, char *tmp_str, uchar max_len);
  void EDITOR::show_message (char **buffer, word delta);
  void EDITOR::choose_color (char *s, byte &color);
  void EDITOR::special_char (byte c, msghdr_type *msghdr);
public:
  EDITOR (void);
  ~EDITOR (void);
  void edit_attributes (dword *attrib);
  char edit_header (msghdr_type *msghdr);
  char *edit_message (char *old_message, msghdr_type *msghdr, byte flags);
  void import_message (char **buffer, word delta, word &lines);
  void fattach(msghdr_type *msghdr);
  void help (void);
};

#endif /* __cplusplus */
#endif /* __EDITOR_H__ */
