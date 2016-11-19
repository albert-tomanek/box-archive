/* A struct used to hold metadata			*
 * about a filesystem entry (file or dir),	*
 * in a BOX archive. Used by ba_EntryList.	*/

#ifndef __BOX_ARCHIVE_ENTRY_H__
  #define __BOX_ARCHIVE_ENTRY_H__

  #include "types.h"

  enum ba_EntryType {
	  ba_EntryType_FILE,
	  ba_EntryType_DIR
  };

  struct ba_Entry {
	  char *path;
	  enum  ba_EntryType type;

	  offset_t __size;		/* <- These store the start and size of the file in the data chunk 	*/
	  offset_t __start;		/*    Not to be tampered with by anything else then the ba library, else you'll lose the reference to your file!	*/
  };

  typedef struct ba_Entry ba_Entry;

  /* A free function */
  void ba_entry_free(ba_Entry *file);

#endif
