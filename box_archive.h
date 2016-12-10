#ifndef __BOX_ARCHIVE_H__
  #define __BOX_ARCHIVE_H__

  #include <stdio.h>
  #include <stdint.h>

  #include "types.h"
  #include "entry.h"

  #define BA_MAX_VER 1		/* The highest box archive version that the program supports */
  #define BA_SEP "/"		/* The file path separator */

  #define NODEBUG			/* Stops debug output */

  /* Structs */
  struct BoxArchive {
      char *loc;
	  char *header;

	  FILE *file;

	  ba_Entry *entry_list;		/* The entry tree with file, dirs, and their metadata */
  };

  typedef struct BoxArchive BoxArchive;


  /* Functions */
  BoxArchive* 	ba_new();
  BoxArchive* 	ba_open(char *loc);           /* The uint8_t is used to store a boolean value */
  void			ba_close(BoxArchive *arch);

  ba_Entry*	ba_get_entries(BoxArchive *arch);	/* Returns a pointer to the archive's entry tree */

  int 		ba_extract(BoxArchive *arch, ba_Entry *file_entry, char *dest);		/* Extract the file at the given path in the given archive, to the given place in the filesystem. Returns 1 if an error occured. */

  void 		ba_debug(BoxArchive *arch, uint8_t debug);	/* Toggle debug output */

  char*  	ba_get_header(BoxArchive *arch);			/* Returns pointer to heap; don't forget to free() it */

  uint8_t 	ba_get_format(BoxArchive *arch);     /* Returns the format version of the given archive, and 0 if the format is invalid.*/

#endif
