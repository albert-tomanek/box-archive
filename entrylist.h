/* Copyright (C) 2017  Albert Tomanek *
 * For license see LICENSE.txt        */

/* Just a simple linked list of filesystem entry structs */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __BOX_ARCHIVE_ENTRYLIST_H__
  #define __BOX_ARCHIVE_ENTRYLIST_H__

  #include "entry.h"
  #include "box_archive.h"

  void 		bael_add	(ba_Entry **first_entry, ba_Entry *add_entry);
  void      bael_remove (BoxArchive *arch, ba_Entry *rm_entry);
  ba_Entry*	bael_get	(ba_Entry *first_entry, char *path);				/* Return a pointer to the struct with the given path. *Does* work with entry trees now. */
  int  		bael_count	(ba_Entry *first);
  void 		bael_free	(ba_Entry **first_entry);
  
  ba_Entry* __bael_getlast(ba_Entry *first);
  ba_Entry* __bael_getprev(ba_Entry *first_entry, ba_Entry *next_entry);
  
#endif

#ifdef __cplusplus
}
#endif
