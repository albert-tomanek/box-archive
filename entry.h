/* A struct used to hold metadata			*
 * about a filesystem entry (file or dir),	*
 * in a BOX archive.						*/

#ifndef __BOX_ARCHIVE_ENTRY_H__
  #define __BOX_ARCHIVE_ENTRY_H__

  #include "types.h"
  #include "file.h"

  enum ba_EntryType {
	  ba_EntryType_FILE,
	  ba_EntryType_DIR
  };

  struct ba_Entry {
	  enum  ba_EntryType type;

	  /* Metadata */
	  char *path;		/* The full path (eg. "/tmp/myProg/file.dat"). WILL contain a '/' at the end if it is a directory.	*/
	  char *name;		/* The file name (eg. "file.dat")				*/

	  /* Pointers */
	  struct ba_File  *file_data;		/* Start and length of the file. NULL if the entry is not a file */
	  struct ba_Entry *parent_dir;			/* Pointer to parent directory. NULL if in root */
	  struct ba_Entry *child_entries;	/* Pointer to the first file in this directory. NULL if this entry is not a directory. */

	  struct ba_Entry *next;				/* Pointer to the next entry */
  };

  typedef struct ba_Entry ba_Entry;

  /* A free function */
  void ba_entry_free(ba_Entry **entry);

#endif
