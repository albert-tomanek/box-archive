#include <stdlib.h>
#include "file.h"

void ba_file_free(ba_File *file)
{
	free(file->path);	/* file->path should *hopefully* be on heap */
	
	free(file);
}