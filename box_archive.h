/* Copyright (C) 2017  Albert Tomanek *
 * For license see LICENSE.txt        */

#ifndef __BOX_ARCHIVE_H__
  #define __BOX_ARCHIVE_H__

  #include <stdio.h>
  #include <stdint.h>

  #include "types.h"
  #include "entry.h"
  #include "ezxml/ezxml.h"

  #define BA_MAX_VER 1		/* The highest box archive version that the program supports */
  #define BA_FMT_VER 0x01	/* The version of the box archive format that this library creates. [Please keep this in hex] */
  #define BA_SEP "/"		/* The file path separator */
  #define BA_INTLEN 20      /* The length of temporary char arrays into which int attributes will be loaded while parsing an archive's XML header */

  #define NODEBUG			/* Stops debug output */
  /*#define RELEASE*/

  #ifdef  RELEASE
    #undef  log_err			/* Don't show debug errors in the release binary */
    #define log_err(...)
	#undef  log_warn
	#define log_warn(...)
  #endif
  
  #ifdef __cplusplus
  extern "C" {
  #endif

  /* Structs */
  struct BoxArchive {
      char *loc;			/* The location of the open archive (if open) */
	  char *header;

	  FILE *file;

	  ba_Entry *entry_tree;		/* The entry tree with file, dirs, and their metadata */
	  fsize_t  __data_size;		/* The size of the whole data chunk in total. Data chunk doesn't exist if *entry_tree is NULL.*/
	  							/* NOTE: While the archive is open, this stores the size of all files INCLUDING the ones that are not stored in buffer but are referred to by entry->__orig_loc. Functions that increment this value: ba_add_file(),  */
  };

  typedef struct BoxArchive BoxArchive;
  typedef struct ezxml ezxml;				/* Not properly implemented in EzXML for some reason... */

  /* Functions */
  BoxArchive* 	ba_new();
  BoxArchive* 	ba_open(char *loc);           /* The uint8_t is used to store a boolean value */
  void			ba_save(BoxArchive *arch, char *loc);
  void			ba_close(BoxArchive *arch);
  ba_Entry*		ba_get_entries(BoxArchive *arch);			/* Returns a pointer to the archive's entry tree */
  ba_Entry*		ba_get(BoxArchive *arch, char *path);		/* Finds the entry with the given path, and returns a pointer to it. */

  void 		ba_add		(BoxArchive *arch, ba_Entry **parent_entry, ba_Entry *add_entry);				/* Adds 'add_entry' to the directory 'parent_entry'. Note: it's prefered to use ba_add_file() and ba_add_dir() instead. */
  ba_Entry*	ba_add_file	(BoxArchive *arch, ba_Entry **parent_entry, char *file_name, char *loc);		/* Adds the file at 'loc' on the current filesystem as 'file_name' in 'parent_directory'. If the file is to be added to the toplevel direcotry, 'parent_directory' should be NULL. If 'loc' is NULL then it creates a zero-byte file. Returns a pointoer to the newly created entry. DO NOT FREE THIS POINTER. */
  ba_Entry*	ba_add_dir	(BoxArchive *arch, ba_Entry **parent_entry, char *dir_name);					/* Like ba_add_file but for a directory. */
  void		ba_move		(BoxArchive *arch, ba_Entry  *src_entry, ba_Entry **dest_entry);
  void 		ba_remove	(BoxArchive *arch, ba_Entry **rm_entry);					/* Delete a file from an archive */

  uint8_t*	ba_get_file_contents	(BoxArchive *arch, ba_Entry *entry, fsize_t *size);		/* Loads the file into buffer and returns the pointer to A COPY OF its data. sets *size to the size/length of the file data. Both this and its sister return a pointer to heap and must be freed. */
  char*		ba_get_textfile_contents(BoxArchive *arch, ba_Entry *entry);					/* A wrapper for ba_get_file_contents() for textfiles. */
  void		ba_set_file_contents	(BoxArchive *arch, ba_Entry *entry, uint8_t *data, fsize_t size);	/* Replaces the data of an existing file entry with the data pointed to. */

  int 		ba_extract(BoxArchive *arch, ba_Entry *file_entry, char *dest);		/* Extract a SINGLE file at the given path in the given archive, to the given place in the filesystem. Returns 1 if an error occured. */

  fsize_t   ba_treesize(ba_Entry *start_entry);			/* Returns the size of all the files in the tree given to it */
  char*  	ba_get_header(BoxArchive *arch);			/* Returns pointer to heap; don't forget to free() it */
  uint8_t 	ba_get_format(BoxArchive *arch);			/* Returns the format version of the given archive, and 0 if the format is invalid.*/

  /* Private stuff */

  void      __ba_load_dir_tree      (fsize_t *total_size, char *orig_header, ba_Entry **first_entry);
  void      __ba_process_xml_dir    (fsize_t *total_size, ezxml *parent_xml, ba_Entry **first_entry, ba_Entry *parent_dir);	/* parent_dir=NULL if toplevel dir */
  ba_Entry* __ba_get_file_metadata  (ezxml *file_node, ba_Entry *parent_dir);
  ba_Entry* __ba_get_dir_metadata   (ezxml *dir_node, ba_Entry *parent_dir);
  ba_Entry* __ba_get_rec_func       (ba_Entry *first_entry, char *path);
  void      __ba_buffer_entries     (BoxArchive *arch);
  void      __ba_buffer_dir         (BoxArchive *arch, ba_Entry *first_entry);
  void      __ba_buffer_file        (BoxArchive *arch, ba_Entry *entry);
  void      __ba_treesize_rec_func  (ba_Entry *first_entry, fsize_t *size);
  void      __ba_dir_entry_to_xml   (ezxml *parent_node, ba_Entry *first_entry);
  void      __ba_create_header      (BoxArchive *arch);
  int       __ba_create_archive_file(BoxArchive *arch, char *loc);	/* Returns 0 on sucess, and > 0 on failure */
  void      __ba_save_entry_dir     (ba_Entry *first_entry, FILE *infile, FILE *outfile, fsize_t *total_size);
  void      __ba_rearrange          (BoxArchive *arch);
  void      __ba_rec_rearrange_func (ba_Entry *first_entry, offset_t *data_size);

  char*     __ba_load_header (FILE* file);
  hdrlen_t  __ba_get_hdrlen  (FILE* file);	/* Used only by ba_load(), and not by ba_save().	*/

  void      __fgetstrn(char *dest, int length, FILE* file);
  
  #ifdef __cplusplus
  }
  #endif

#endif
