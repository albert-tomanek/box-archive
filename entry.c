#include <stdlib.h>
#include "entry.h"

void ba_entry_free(ba_Entry *entry)
{
	free(entry->path);		/* entry->path should *hopefully* be on heap */

	free(entry);
}
