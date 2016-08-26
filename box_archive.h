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
	  
	  /* private stuff */
	  uint8_t __debug;
  };

  typedef struct BoxArchive BoxArchive;

  /* Functions */
  BoxArchive* 	ba_open(char *loc);           /* The uint8_t is used to store a boolean value */
  void			ba_close(BoxArchive *arch);
  
  void 		ba_debug(BoxArchive *arch, uint8_t debug);
  
  int 		ba_get_hdrlen(BoxArchive *arch);
  #define 	ba_get_header_length(...) ba_get_hdrlen(##__VA_ARGS__)
  
  char*  	ba_get_header(BoxArchive *arch);
  #define 	ba_get_hdr(...) ba_get_header(##__VA_ARGS__)
  
  uint8_t 	ba_get_format(BoxArchive *arch);     /* Returns the format version of the given archive, and 0 if the format is invalid.*/
  #define 	ba_get_fmt(...) ba_get_format(##__VA_ARGS__)
  
#endif
