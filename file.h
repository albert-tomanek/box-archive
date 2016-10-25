/* A struct used to hold metadata
 * about a file in a BOX archive.
 */


#ifndef __BOX_ARCHIVE_FILE_H__
  #define __BOX_ARCHIVE_FILE_H__
  
  #include "types.h"
  
  enum ba_FileType {
	  ba_FileType_FILE,
	  ba_FileType_DIR
  };
  
  struct ba_File {
	  char *path;
	  enum  ba_FileType type;
	  
	  offset_t __size;		/* <- These store the start and size of the file in the data chunk 	*/
	  offset_t __start;		/*    Not to be tampered with by anything else then the ba library, else you'll lose the reference to your file!	*/
  };
  
  typedef struct ba_File ba_File;
  
#endif