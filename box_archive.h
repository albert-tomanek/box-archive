#ifndef __BOX_ARCHIVE_H__
  #define __BOX_ARCHIVE_H__
  
  #include <stdio.h>
  
  /* Debug options */
  #define NO_DEBUG  0
  #define DEBUG     1
  /* Structs */
  
  struct BoxArchive {
      char *loc;
	  char *header;
	  FILE *file;
  };
  
  typedef struct BoxArchive BoxArchive;
  
  /* Functions */
  BoxArchive* ba_open(char *loc, char debug);           /* The char is treated as a uint8_t */
  void ba_get_header(BoxArchive *arch, char *out);
  
#endif
