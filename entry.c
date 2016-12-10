#include <stdlib.h>
#include "entry.h"

void ba_entry_free(ba_Entry **entry)
{
	if (! entry) return;

	if ((*entry)->name)	free((*entry)->name);
	if ((*entry)->path)	free((*entry)->path);		/* entry->path should *hopefully* be on heap */

	free((*entry));
	*entry = NULL;
}

char* ba_entry_nice_type(enum ba_EntryType type)
{
	/* returns the type of entry as text */

	if (type == ba_EntryType_FILE)	return "File";
	if (type == ba_EntryType_DIR)	return "Directory";

	return "Undefined";
}
