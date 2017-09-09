/* configuration.h - definition of the configuration class of tanstaafl */
/* Copyright 1998-1999 Per Lundberg */

#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__

#define MAX_AREAS 1024

#ifdef __cplusplus
extern "C"
{
#endif
typedef struct
{
  char signature[11];      /* Should be "tanstacfg", 0x1B */
  word version;            /* Lo is minor, hi is major. Current is 1.0 */
  char point_name[65];     /* The name of the point */
  char user_name[65];      /* The name of the user */
  char location[65];       /* The location of the point */
  address_type aka[10];    /* The AKA:s of this point */
  address_type uplink[10]; /* Our uplinks */
  char password[10][9];    /* Used as packet, areafix and session password */
  char system_path[128];   /* The main system directory */
  char msgbase_path[128];  /* The location of the message base files */
  char inbound_path[128];  /* The location of the inbound directory */
  char outbound_path[128]; /* The base outbound directory */
  char temp_path[128];     /* The location of temporary files */
  byte unused[6];          /* used to be colours (don't ask...) */
  dword quote_type: 2;     /* 0 = '>', 1 = 'xy>', 2 = 'XY>' */
  dword mailer: 1;         /* 0 = BinkleyTerm/Xenia, 1 = ifcico */
  dword confirm_exit: 1;   /* 1 = confirm on program exit */
  dword jump_to_next: 1;   /* 1 = jump to next area when trying to go past the end of the current area */
  dword separating_line: 1;/* 1 = show separating line in reader */
  dword scan_on_exit: 1;   /* 1 = scan for outgoing mail on exit */
  dword reserved: 25;      /* reserved for future use */
  char phone[10][40];      /* Phone numbers or host names */
  char poll_cmd[128];      /* Name of external mailer */
  byte color[64];         /* Color configuration */
} __attribute__ ((packed)) configuration_type;

typedef struct
{
  char signature[12];      /* should be "tanstareas", 0x1B */
  word version;            /* Lo is minor, hi is major */
  word areas;              /* How many areas are there in this configuration file? */
  word groups;             /* How many groups are there in this configuration file? */
} __attribute__ ((packed)) areaheader_type;

typedef struct
{
  char name[80];           /* The tag name of the area */
  char comment[80];        /* A comment describing the area */
  char aka;                /* Which address should we use for this area? */
  char in_charset;         /* Incoming texts are translated from this charset */
  char out_charset;        /* Outgoing texts are translated to this charset */
  byte uplink;             /* What uplink should be used for this area? */
  char reserved[127];      /* Reserved for future use */

  struct attribute
  {
    byte no_reply: 1;      /* Is this area reply-protected? */
    byte no_original: 1;   /* Is this area reply-only? */
    byte link: 1;          /* Should this area be reply linked? */
    byte throw_comments: 1;/* If set, comments placed in this area will be
			      thrown to reply_area when tossing */
    byte sort: 1;          /* Should this area be sorted on date? */
  };
  byte unused;
  word reply_area;         /* In which area should replies be placed? */
  sword origin;            /* Which origin should be the default for this 
			      area? -1 for random. */
} __attribute__ ((packed)) area_type;
#ifdef __cplusplus
};
#endif /* __cplusplus */

#ifdef __cplusplus

class CONFIG
{
public:
  area_type *area[MAX_AREAS];
  configuration_type cfg;
  areaheader_type area_header;
  bool color;
  
  void play (void);
  void configure_akas (void);
  void configure_area (word);
  void configure_areas (void);
  void configure_colors (void);
  void choose_color_scheme (void);
  void configure_reader_colors (void);
  void configure_misc_colors (void);
  void configure_editor_colors (void);
  void configure_directories(void);
  void configure_misc (Window *win);
  void configure_site_info (void);
  void configure_uplinks (void);
  void configure_passwords (void);
  void configure_phone_numbers (void);
  void read_address (byte x, byte y, address_type *address);
  void edit_uplink (Window *win, int which);
  void read_areas (void);
  void read (void);
  void write_areas (void);
  void write (void);
  word add_area (area_type new_area);
};

#endif /* __cplusplus */
#endif /* __CONFIGURE_H__ */
