#include <stdlib.h>
#include "entry.h"

void ba_entry_free(ba_Entry *entry)
{
	if (! entry) return;

	if (entry->name)	free(entry->name);
	if (entry->path)	free(entry->path);		/* entry->path should *hopefully* be on heap */

	free(entry);
}
