#include <stdio.h>
#include <stdlib.h>

#include "box_archive.h"
#include "errors.h"
#include "ints.h"

BoxArchive* ba_open(char *loc, char debug)
{
	BoxArchive *arch = malloc(sizeof(BoxArchive));

	if (!arch) {
		error(ERR_MEM, "[ERROR] Out of memory.\n");
	}
	if (debug)  {
		printf("[DEBUG] Opening file %s...\t", loc);
	}

	arch->file = fopen(loc, "rw");

	if (! arch->file) {
		error(ERR_FOPEN, "[ERROR] The file could not be opened.\n");
	}
	if (debug) {
		printf("done.\n");
	}

	return arch;
}

void __ba_gethdr(BoxArchive *arch, char *out)
{

}
