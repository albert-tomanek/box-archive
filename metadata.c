#include <stdlib.h>
#include <time.h>

#include "dbg.h"
#include "metadata.h"

ba_Meta *ba_meta_default()
{
	ba_Meta *meta = malloc(sizeof(ba_Meta));
	check(meta, "Out of memory.");

	meta->atime = time(NULL);	// Set the access and modification time to now
	meta->mtime = time(NULL);
	
	return meta;
	
error:
	return NULL;
}

void ba_meta_free(ba_Meta *meta)
{
	check(meta, "Null-pointer passed to ba_meta_free().");

	free(meta);

error:
	return;
}
