 /* A struct used to hold metadata			*
  * about a filesystem entry (file or dir),	*
  * in a BOX archive.						*/

#ifndef __BOX_ARCHIVE_ENTRY_H__
  #define __BOX_ARCHIVE_ENTRY_H__

  #include "types.h"
  #include "file.h"

  enum ba_EntryType {
	  be_EntryType_UNSPECIFIED = 0,
	  ba_EntryType_FILE,
	  ba_EntryType_DIR
  };

  struct ba_Entry {
	  enum  ba_EntryType type;

	  char *__orig_loc;		/* If this entry is a file, this will contain the location of the original file, so that the ba_save() knows where to read the file's data from when creating the archive. */

	  /* Metadata */
	  char *path;		/* The full path (eg. "/tmp/myProg/file.dat"). WILL contain a '/' at the end if it is a directory. The string will be on heap and therefore will be freed when ba_entry_free() is called. */
	  char *name;		/* The file name (eg. "file.dat"). Will also be on heap.	*/

	  /* Pointers */
	  struct ba_File  *file_data;		/* Start and length of the file. NULL if the entry is not a file */
	  struct ba_Entry *parent_dir;		/* Pointer to parent directory. NULL if in root */
	  struct ba_Entry *child_entries;	/* Pointer to the first file in this directory. NULL if this entry is not a directory. */

	  struct ba_Entry *next;				/* Pointer to the next entry */
  };

  /* (Sorry -- this is not the most trlevant pcace to put this...)
   * When saving an entry:																		*
   *  - If the file has just been added and isn't in the archive file yet, 						*
   *	its content is obtained from ->__orig_loc												*
   *  - If the file has been changed since loaded, or the source archive is being overwritten,	*
   *	its content is obtained from ->file_data->buffer										*
   *  - If the file has not been modified since it was read, ->__orig_loc and ->file_data->buffer*
   *	are null-pointers and its data is obtained from the data chunk in arch->file at 		*
   *	the offset of ->file_data->__start.														*
   *																							*
   * arch->__data_size gets incremented by:														*
   *  - Adding a file with ba_add_file()														*
   *  - __ba_process_xml_dir() when loading an archive											*
   *  - __rec_getdir_func() [from ba_load_fs_tree()] when loading a filesystem into the entry tree	*
   *																							*/

  typedef struct ba_Entry ba_Entry;

  /* Functions */
  void  ba_entry_free(ba_Entry *entry);
  const char* ba_entry_nice_type(enum ba_EntryType type);
  const char* ba_entry_xml_type (enum ba_EntryType type);

#endif
