#ifndef __BOX_ARCHIVE_H__
  #define __BOX_ARCHIVE_H__

  #include <stdio.h>
  #include <stdint.h>

  #include "types.h"
  #include "entry.h"

  #define BA_MAX_VER 1		/* The highest box archive version that the program supports */
  #define BA_FMT_VER 0x01	/* The version of the box archive format that this library creates. [Please keep this in hex]*/
  #define BA_SEP "/"		/* The file path separator */
  #define BA_INTLEN 20      /* The length of temporary char arrays into which int attributes will be written */

  #define NODEBUG			/* Stops debug output */

  /* Structs */
  struct BoxArchive {
      char *loc;			/* The location of the open archive (if open) */
	  char *header;

	  FILE *file;

	  ba_Entry *entry_list;		/* The entry tree with file, dirs, and their metadata */
	  fsize_t  __data_size;		/* The size of the whole data chunk in total. Data chunk doesn't exisi if *entry_list is NULL.*/
	  							/* NOTE: While the archive is open, this stores the size of all files INCLUDING the ones that are not stored in buffer but are referred to by entry->__orig_loc. Functions that increment this value: ba_add_file(),  */
  };

  typedef struct BoxArchive BoxArchive;


  /* Functions */
  BoxArchive* 	ba_new();
  BoxArchive* 	ba_open(char *loc);           /* The uint8_t is used to store a boolean value */
  void			ba_save(BoxArchive *arch, char *loc);
  void			ba_close(BoxArchive *arch);

  ba_Entry*	ba_get_entries(BoxArchive *arch);			/* Returns a pointer to the archive's entry tree */
  ba_Entry* ba_get(BoxArchive *arch, char *path);		/* Finds the entry with the given path, and returns a pointer to it. */

  void 		ba_add		(BoxArchive *arch, ba_Entry **parent_entry, ba_Entry *add_entry);				/* Adds 'add_entry' to the directory 'parent_entry'. Note: it's prefered to use ba_add_file() and ba_add_dir() instead. */
  void 		ba_add_file	(BoxArchive *arch, ba_Entry **parent_entry, char *file_name, char *loc);
  void 		ba_add_dir	(BoxArchive *arch, ba_Entry **parent_entry, char *dir_name);
  void 		ba_remove	(BoxArchive *arch, ba_Entry **rm_entry);					/*  */

  int 		ba_extract(BoxArchive *arch, ba_Entry *file_entry, char *dest);		/* Extract the file at the given path in the given archive, to the given place in the filesystem. Returns 1 if an error occured. */

  char*  	ba_get_header(BoxArchive *arch);			/* Returns pointer to heap; don't forget to free() it */
  uint8_t 	ba_get_format(BoxArchive *arch);			/* Returns the format version of the given archive, and 0 if the format is invalid.*/

#endif
