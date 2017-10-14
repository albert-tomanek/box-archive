/* Copyright (C) 2017  Albert Tomanek *
 * For license see LICENSE.txt        */

/* A struct used to hold metadata			*
 * about a filesystem entry (file or dir),	*
 * in a BOX archive.						*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __BOX_ARCHIVE_ENTRY_H__
  #define __BOX_ARCHIVE_ENTRY_H__

  #include "types.h"
  #include "metadata.h"
  #include "file.h"

  enum ba_EntryType {
	  be_EntryType_UNSPECIFIED = 0,
	  ba_EntryType_FILE,
	  ba_EntryType_DIR
  };

  struct ba_Entry {
	  enum  ba_EntryType type;

	  char *__orig_loc;		/* If this entry is a file, this will contain the location of the original file, so that the ba_save() knows where to read the file's data from when creating the archive. */
	  						/* If the entry is a directory, it will contain the original location of the directory in the filesystem. This is howeer currently only set and used internally by __rec_getdir_func from the Linux/UNIX section of filesystem.c */

	  /* Metadata */
	  char *path;		/* The full path (eg. "/tmp/myProg/file.dat"). WILL contain a '/' at the end if it is a directory. The string will be on heap and therefore will be freed when ba_entry_free() is called. */
	  char *name;		/* The file name (eg. "file.dat"). Will also be on heap.	*/

	  ba_Meta *meta;	/* Struct containing all other metadata like access times and permissions */

	  /* Pointers */
	  struct ba_File  *file_data;		/* Start and length of the file. NULL if the entry is not a file */
	  struct ba_Entry *parent_dir;		/* Pointer to parent directory. NULL if in root */
	  struct ba_Entry *child_entries;	/* Pointer to the first file in this directory. NULL if this entry is not a directory. */

	  struct ba_Entry *next;				/* Pointer to the next entry */
  };

  typedef struct ba_Entry ba_Entry;

  /* Functions */
  void  ba_entry_free(ba_Entry *entry);
  const char* ba_entry_nice_type(enum ba_EntryType type);
  const char* ba_entry_xml_type (enum ba_EntryType type);

#endif

#ifdef __cplusplus
}
#endif
