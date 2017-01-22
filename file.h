/* A struct used to hold internal (to the program)	*
 * data about a file (such as start and size).		*
 * A struct containing the metadata is pointed to.	*/

#include <stdint.h>

#ifndef __BOX_ARCHIVE_FILE_H__
  #define __BOX_ARCHIVE_FILE_H__

  #include "types.h"

  struct ba_File
  {
	uint8_t *buffer;		/* The file's contents as an array of bytes stored on the heap.	*
							 * If the buffer needs to be enlarged, it can be realloc()'ed.	*
							 * Size of array is stored in ->__size.							*/

	offset_t __size;		/* <- These store the start and size of the file in the data chunk 	*/
	offset_t __start;		/*    Not to be tampered with by anything else then the ba library, else you'll lose the reference to your file!	*/
  };

  typedef struct ba_File ba_File;

  void ba_file_free(struct ba_File **file);		/* A free function for our ba_File struct */

#endif
