#ifndef __BOX_ARCHIVE_H__
  #define __BOX_ARCHIVE_H__

  #include <stdio.h>
  #include <stdint.h>
  
  #include "filelist.h"

  #define BA_MAX_VER 1		/* The highest box archive version that the program supports */
  #define BA_SEP "/"		/* The file path separator */
  
  #define NODEBUG			/* Stops debug output */

  /* Structs */
  struct BoxArchive {
      char *loc;
	  char *header;
	  
	  FILE *file;
	  
	  ba_FileList *file_list;		/* Files and their metadata */
	  
  };
  
  typedef struct BoxArchive  BoxArchive;

  
  /* Functions */
  BoxArchive* 	ba_open(char *loc);           /* The uint8_t is used to store a boolean value */
  void			ba_close(BoxArchive *arch);
  
  ba_FileList*  ba_get_files(BoxArchive *arch);
  #define   	ba_getfiles(A)	ba_get_files(A)
  
  void 		ba_debug(BoxArchive *arch, uint8_t debug);	/* Toggle debug output */
  
  char*  	ba_get_header(BoxArchive *arch);			/* Returns pointer to heap; don't forget to free() it */
  #define 	ba_gethdr(A)	ba_get_header(A)
  
  uint8_t 	ba_get_format(BoxArchive *arch);     /* Returns the format version of the given archive, and 0 if the format is invalid.*/
  #define 	ba_getfmt(A)	ba_get_format(A)
  
  void ba_list(BoxArchive *arch);
  
#endif
