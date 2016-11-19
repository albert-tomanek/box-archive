#include <string.h>
#include <stdlib.h>

#include "dbg.h"
#include "entry.h"
#include "entrylist.h"

/* Private stuff */

ba_EntryList* __bael_getlast(ba_EntryList *first);

/*  */

void bael_add(ba_EntryList **first_entry, ba_Entry *entry)
{
	ba_EntryList *new_entry = malloc(sizeof(ba_EntryList));

	new_entry->next  = NULL;
	new_entry->entry = entry;

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

void bael_free(ba_EntryList **first_entry)	/* Double-pointer because we will be changing the original pointer to NULL */
{
	ba_EntryList *current_entry = *first_entry;
	ba_EntryList *next;

	if (! current_entry)		/* In case first_entry is NULL or we are on the last entry */
		return;

	while(current_entry)
	{
		next = current_entry->next;

		ba_entry_free(current_entry->entry);	/* Free the ba_Entry struct that it points to. */
		free         (current_entry);

		current_entry = next;
	}

	*first_entry = NULL;		/* Change it to null since the current pointer would be invalid */

	return;
}

int bael_count(ba_EntryList *first)
{
	int count = 0;
	ba_EntryList *current = first;

	while (current)
	{
		count++;
		current = current->next;
	}

	return count;
}

ba_Entry* bael_get(ba_EntryList *first_entry, char *path)
{
	ba_EntryList* current = first_entry;

	while (current)
	{
		if (! strcmp(path, current->entry->path))
		{
			return current->entry;
		}

		current = current->next;
	}

	return NULL;
}

ba_EntryList* __bael_getlast(ba_EntryList *first)
{
	check(first, "Null pointer passed to __bael_getlast()");

	ba_EntryList *current = first;

	while (current->next)
	{
		current = current->next;
	}

	return current;

error:
	return first;
}
