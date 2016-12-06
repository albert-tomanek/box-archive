/* A struct used to hold internal (to the program)	*
 * data about a file (such as start and size).		*
 * A struct containing the metadata is pointed to.	*/

#include <stdint.h>

#include "box_archive.h"
#include "entry_list.h"

#ifndef __BOX_ARCHIVE_FILE_H__
  #define __BOX_ARCHIVE_FILE_H__

  struct ba_File
  {
	BoxArchive   *archive;
	ba_EntryList *meta;		/* meta->entry contains the file's metadata */	/* Will in future be changed to ba_Entry once it gains ->next */

	uint8_t *contents;		/* The file's contents as an array of bytes stored on the heap.	*
							 * If the file contents change, then these can be free()'d and	*
							 * replaced. Can be set to NULL to indicate that the file's 	*
							 * contents weren't changed.									*/

	offset_t __size;		/* <- These store the start and size of the file in the data chunk 	*/
	offset_t __start;		/*    Not to be tampered with by anything else then the ba library, else you'll lose the reference to your file!	*/
  };



#endif
