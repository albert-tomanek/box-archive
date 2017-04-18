/* Copyright (C) 2017  Albert Tomanek *
 * For license see LICENSE.txt        */

#include <string.h>
#include <stdlib.h>

#include "dbg.h"
#include "box_archive.h"
#include "file.h"
#include "entry.h"
#include "entrylist.h"

/* Private stuff */

ba_Entry* __bael_getlast(ba_Entry *first);
ba_Entry* __bael_getprev(ba_Entry *first_entry, ba_Entry *next_entry);

/*  */

void bael_add(ba_Entry **first_entry, ba_Entry *new_entry)
{
	new_entry->next  = NULL;

	if (*first_entry == NULL)
	{
		/* If the list is empty */

		*first_entry = new_entry;
	}
	else
	{
		/* Add it to the end */
		__bael_getlast(*first_entry)->next = new_entry;
	}

	return;
}

void bael_remove(BoxArchive *arch, ba_Entry *rm_entry)
{
	/* Removes rm_entry (if present) from parent_dir->child_entries.	*
	 * Does NOT free rm_entry (just removes it).						*/

	check(arch, "Null-pointer given for *arch to bael_remove().");
	check(rm_entry  , "Null-pointer given for *rm_entry to bael_remove().");

	ba_Entry **first_entry = (rm_entry->parent_dir != NULL) ? &(rm_entry->parent_dir->child_entries) : &(arch->entry_tree);		/* If an entry is in the toplevel directory, *first_entry is arch->entry_tree */

	if (*first_entry == rm_entry)
	{
		/* If we are the first in the list */

		*first_entry = rm_entry->next;
	}
	else
	{
		ba_Entry *prev_entry = __bael_getprev(*first_entry, rm_entry);
		check(prev_entry, "__bael_getprev() returned NULL.");

		prev_entry->next = rm_entry->next;
	}

	rm_entry->next = NULL;

	return;

error:

	return;
}

void bael_free(ba_Entry **first_entry)	/* Double-pointer because we will be changing the original pointer to NULL */
{
	/* Frees the whole tree of entries given to it. */

	ba_Entry *current_entry = *first_entry;
	ba_Entry *next;

	if (! current_entry)		/* In case first_entry is NULL or we are on the last entry */
		return;

	while (current_entry)
	{
		next = current_entry->next;

		if (current_entry->file_data)		ba_file_free(&(current_entry->file_data));
		if (current_entry->child_entries)	bael_free(   &(current_entry->child_entries));		/* Free the child entries */
		ba_entry_free(current_entry);	/* Free the ba_Entry struct that it points to. */

		current_entry = next;
	}

	*first_entry = NULL;		/* Change it to null since the current pointer would be invalid */

	return;
}

int bael_count(ba_Entry *first)
{
	int count = 0;
	ba_Entry *current = first;

	while (current)
	{
		count++;
		current = current->next;
	}

	return count;
}

ba_Entry* bael_get(ba_Entry *first_entry, char *path)
{
	ba_Entry* current = first_entry;

	while (current)
	{
		if (! strcmp(path, current->path))
		{
			return current;
		}

		current = current->next;
	}

	return NULL;
}

ba_Entry* __bael_getprev(ba_Entry *first_entry, ba_Entry *next_entry)
{
	check(first_entry, "Null-pointer given for *first_entry to __bael_getprev().");
	check(next_entry, "Null-pointer given for *next_entry to __bael_getprev().");

	if (next_entry == first_entry)
	{
		/* If we are the first in the list */
		return first_entry;
	}

	ba_Entry *current = first_entry;

	while (current->next != next_entry && current->next != NULL)
	{
		current = current->next;
	}

	return current;

error:
	return NULL;
}


ba_Entry* __bael_getlast(ba_Entry *first)
{
	check(first, "Null pointer passed to __bael_getlast()");

	ba_Entry *current = first;

	while (current->next)
	{
		current = current->next;
	}

	return current;

error:
	return first;
}
