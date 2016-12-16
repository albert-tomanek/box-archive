#include <stdlib.h>
#include "entry.h"

void ba_entry_free(ba_Entry *entry)
{
	if (! entry) return;

	if (entry->name)	free(entry->name);
	if (entry->path)	free(entry->path);		/* entry->path should *hopefully* be on heap */

	free(entry);
	/* *entry = NULL; */
}

char* ba_entry_nice_type(enum ba_EntryType type)
{
	/* Returns the type of entry as text.					*
	 * Note: These are constants, and not on the stack,		*
	 * so we don't need to worry about them getting cleared	*
	 * after the end of the function...						*/

	if (type == ba_EntryType_FILE)	return "File";
	if (type == ba_EntryType_DIR)	return "Directory";

	return "Undefined";
}
