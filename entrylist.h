/* Just a simple linked list of filesystem entry structs,			*
 * passed from the ba library to the program using it.	*/

#ifndef __BOX_ARCHIVE_ENTRYLIST_H__
  #define __BOX_ARCHIVE_ENTRYLIST_H__

  #include "entry.h"

  void 		bael_add	(ba_Entry **first_entry, ba_Entry *entry);
  ba_Entry*	bael_get	(ba_Entry *first_entry, char *path);				/* Return a pointer to the struct with the given path */
  int  		bael_count	(ba_Entry *first);
  void 		bael_free	(ba_Entry **first_entry);

#endif
