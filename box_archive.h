#ifndef __BOX_ARCHIVE_H__
  #define __BOX_ARCHIVE_H__

  #include <stdio.h>
  #include <stdint.h>

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
  BoxArchive* ba_open(char *loc, uint8_t debug);           /* The uint8_t is used to store a boolean value */
  void ba_get_header(BoxArchive *arch, char *out);
  uint8_t ba_get_format(BoxArchive *arch);     /* Returns the format version of the given archive, and 0 if the format is invalid.*/
  void ba_close(BoxArchive *arch);

#endif
