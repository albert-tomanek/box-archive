#include <stdlib.h>
#include "entry.h"

void ba_file_free(struct ba_File **file)
{
	if (! file)
		return;

	if ((*file)->contents)	free((*file)->contents);
	free(*file);

	*file = NULL;
	return;
}
