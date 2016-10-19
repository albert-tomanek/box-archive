#ifndef __BOX_ARCHIVE_H__
  #define __BOX_ARCHIVE_H__

  #include <stdio.h>
  #include <stdint.h>

  #define BA_MAX_VER 1		/* The highest box archive version that the program supports */
  #define BA_SEP "/"		/* The file path separator */

  /* Structs */
  struct BoxArchive {
      char *loc;
	  FILE *file;
	  
	  /* private stuff */
	  uint8_t __debug;
  };

  typedef struct BoxArchive BoxArchive;

  /* Functions */
  BoxArchive* 	ba_open(char *loc);           /* The uint8_t is used to store a boolean value */
  void			ba_close(BoxArchive *arch);
  
  void 		ba_debug(BoxArchive *arch, uint8_t debug);	/* Toggle debug output */
  
  int 		ba_get_hdrlen(BoxArchive *arch);			/* get the length of the header in bytes */
  #define 	ba_get_header_length(...) ba_get_hdrlen(##__VA_ARGS__)
  
  char*  	ba_get_header(BoxArchive *arch);			/* Returns pointer to heap; don't forget to free() it */
  #define 	ba_gethdr(A)	ba_get_header(A)
  
  uint8_t 	ba_get_format(BoxArchive *arch);     /* Returns the format version of the given archive, and 0 if the format is invalid.*/
  #define 	ba_getfmt(...)	ba_get_format(##__VA_ARGS__)
  
  void ba_list(BoxArchive *arch);
  
#endif
