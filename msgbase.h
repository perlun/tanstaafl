/* msgbase.h - definition of the message base class and structures */
/* Copyright 1998-1999 Per Lundberg */

#ifndef __MSGBASE_H__
#define __MSGBASE_H__

#ifdef __cplusplus
#include <string>
#endif

/* Message base flags */

#define FLAG_LOCAL 0x1
#define FLAG_PRIVATE 0x4
#define FLAG_READ 0x8
#define FLAG_SENT 0x10
#define FLAG_DELSENT 0x20
#define FLAG_ATTACH 0x100
#define FLAG_CRASH 0x200
#define FLAG_HOLD 0x400
#define FLAG_SEEN 0x1000
#define FLAG_PGM 0x800000
#define FLAG_LOCKED 0x40000000
#define FLAG_DELETED 0x80000000

#ifdef __cplusplus
extern "C" {
#endif

/* msgstat.tfl */
typedef struct
{
  word msgbase_version;  /* Should be 1 */
  byte reserved[254];
  dword messages[2000];  /* The number of messages of each area */
  dword lastread[2000];  /* The number of the last read message */
  dword unread[2000];    /* The number of unread messages in each area */
} __attribute__ ((packed)) msgstat_type;

/* msghdr.tfl */
typedef struct
{
  byte recipient[36];
  byte sender[36];
  byte subject[72];
  dword msgid_CRC;
  dword reply_CRC;
  dword reply_mother;
  dword reply_next;
  dword reply_prev;
  dword time_written;
  dword reply_child;
  address_type recipient_addr;
  address_type sender_addr;
  dword status_flags;
  dword txt_start;
  dword txt_size;
  word folder;
  word folder_reply_mother;
  word folder_reply_next;
  word folder_reply_prev;
} __attribute__ ((packed)) msghdr_type;

#ifdef __cplusplus
};

typedef class MSGBASE
{
  string msgbase_path;
  word area;          /* The number of the current area */
  dword last_message; /* The lastread message of the area */
 public:
  dword *msg;         /* A list of all the messages this area contains */
  byte *msgtxt;
  dword messages;     /* The number of messages in this area */
  MSGBASE (string msgbase_path);
  MSGBASE (void);
  ~MSGBASE (void);
  bool init (string msgbase_path);
  char open_area (word area_number, dword *last_message);
  void open (dword area_number);
  bool close_area (void);
  char read_message (dword message, msghdr_type *msghdr);
  char read_message_header (dword message, msghdr_type *msghdr);
  char read_message_text (dword message, msghdr_type msghdr);
  bool write_message (dword message, msghdr_type msghdr, char *text);
  dword write_message (msghdr_type msghdr, char *text);
  bool write_message_header (dword message, msghdr_type msghdr);
  bool delete_message (dword message);
  bool get_stats (word area, dword *messages, dword *last, dword *unread);
  bool rebuild_msgstat (void);
  void link (dword start, dword end);
  dword get_number_of_messages (void);
  dword get_new (dword area_number);
  dword linear_to_logic (dword message);
  dword area_number (dword message);
};
#endif /* __cplusplus */


#endif /* __MSGBASE_H__ */
