#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "box_archive.h"
#include "errors.h"
#include "ints.h"

BoxArchive* ba_open(char *loc, uint8_t debug)
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

	if (ba_get_format(arch) == 0)
	{
		error(ERR_FFORMAT, "[ERROR] Not a box archive.");
	}

	return arch;
}

/* If the file is not a BOX archive,
 * 0 will be returned
 */
uint8_t ba_get_format(BoxArchive *arch)
{
	rewind(arch->file);				/* Go to the start of the file */

	uint8_t hdr_bytes[4];	/* THREE cells long */

	hdr_bytes[0] = fgetc(arch->file);
	hdr_bytes[1] = fgetc(arch->file);
	hdr_bytes[2] = fgetc(arch->file);
	hdr_bytes[3] = fgetc(arch->file);
	
	if (hdr_bytes[0] != 0xde || hdr_bytes[1] != 0xca || hdr_bytes[2] != 0xde) {	/* Pure genious */
		return 0;
	} else {
		return hdr_bytes[3];
	}
}

void ba_gethdr(BoxArchive *arch, char *out)
{

}

void ba_close(BoxArchive *arch)
{
	fclose(arch->file);

	free(arch);
}
