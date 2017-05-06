#include <stdlib.h>
#include <time.h>

#include "dbg.h"
#include "metadata.h"

void ba_meta_free(ba_Meta *meta)
{
	check(meta, "Null-pointer passed to ba_meta_free().");

	free(meta);

error:
	return;
}
