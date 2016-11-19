/* Just a simple linked list of filesystem entry structs,			*
 * passed from the ba library to the program using it.	*/

#ifndef __BOX_ARCHIVE_ENTRYLIST_H__
  #define __BOX_ARCHIVE_ENTRYLIST_H__

  #include "entry.h"

  struct ba_EntryList {
      struct ba_Entry     *entry;
      struct ba_EntryList *next;
  };

  typedef struct ba_EntryList ba_EntryList;

  void 		bael_add	(ba_EntryList **first_entry, ba_Entry *entry);
  ba_Entry*	bael_get	(ba_EntryList *first_entry, char *path);				/* Return a pointer to the struct with the given path */
  int  		bael_count	(ba_EntryList *first);
  void 		bael_free	(ba_EntryList **first_entry);

#endif
