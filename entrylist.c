#include <string.h>
#include <stdlib.h>

#include "dbg.h"
#include "file.h"
#include "entry.h"
#include "entrylist.h"

/* Private stuff */

ba_Entry* __bael_getlast(ba_Entry *first);

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
}

void bael_free(ba_Entry **first_entry)	/* Double-pointer because we will be changing the original pointer to NULL */
{
	ba_Entry *current_entry = *first_entry;
	ba_Entry *next;

	if (! current_entry)		/* In case first_entry is NULL or we are on the last entry */
		return;

	while (current_entry)
	{
		next = current_entry->next;

		if (current_entry->file_data)		ba_file_free(&(current_entry->file_data));
		if (current_entry->child_entries)	bael_free(   &(current_entry->child_entries));
		ba_entry_free(&current_entry);	/* Free the ba_Entry struct that it points to. */

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
