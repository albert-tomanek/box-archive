#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "box_archive.h"
#include "errors.h"
#include "positions.h"
#include "ints.h"

void __fgetstrn(char *dest, int length, FILE* file);

BoxArchive* ba_open(char *loc, uint8_t debug)
{
	BoxArchive *arch = malloc(sizeof(BoxArchive));

	if (!arch) {
		error(0, "[ERROR] Out of memory.\n");
		return NULL;
	}
	if (debug)  {
		printf("[DEBUG] Opening file %s...\t", loc);
	}
	
	/* Open the actual file */
	arch->file = fopen(loc, "rw");

	if (! arch->file) {
		error(0, "[ERROR] The file could not be opened.\n");
		return NULL;
	}
	if (debug) {
		printf("done.\n");
	}

	if (ba_get_format(arch) == 0)
	{
		error(0, "[ERROR] Not a box archive.");
		return NULL;
	}

	return arch;
}

/* If the file is not a BOX archive,
 * 0 will be returned
 */
uint8_t ba_get_format(BoxArchive *arch)
{
	if (! arch)
	{
		error(0, "[ERROR] Null-pointer given to ba_get_format().\n");
		return 0;
	}
	if (! arch->file)
	{
		error(0, "[ERROR] File not open!\n");
		return 0;
	}
	
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

int   ba_get_hdrlen(BoxArchive *arch)
{
	if (! arch)
	{
		error(0, "[ERROR] Null-pointer given to ba_get_header().\n");
		return 0;
	}
	if (! arch->file)
	{
		error(0, "[ERROR] FilFilee not open!\n");
		return 0;
	}
	
	/* Go to the two-byte header length */
	fseek(arch->file, P_HEADER_LENGTH, SEEK_SET);
	
	fgetc(arch->file);
	uint8_t byte1	=	fgetc(arch->file);
	uint8_t byte2	=	fgetc(arch->file);
	
	return cvt8to16(byte1, byte2);	/* function from ints.h */
}

char* ba_get_header(BoxArchive *arch, uint8_t debug)
{
	if (! arch)
	{
		error(0, "[ERROR] Null-pointer given to ba_get_header().\n");
		return 0;
	}
	if (! arch->file)
	{
		error(0, "[ERROR] File not open!\n");
		return 0;
	}
	
	/* Go to the header */
	fseek(arch->file, P_HEADER, SEEK_SET);
	
	uint16_t hdr_length = ba_get_hdrlen(arch);
	char* header = calloc(hdr_length+1, sizeof(char));	/* +1 for the null-byte */
	
	if (debug)
		printf("[DEBUG] XML header length = %d bytes\n", hdr_length);
	
	__fgetstrn(header, hdr_length, arch->file);
	
	return header;
}

void ba_close(BoxArchive *arch)
{
	if (! arch)
	{
		error(ERR_NULLPTR, "[ERROR] Null-pointer given to ba_close().\n");
	}
	else
	{
		if (! arch->file)
			fprintf(stderr, "[WARNING] Cannot close archive; archive not open!\n");
		else
			fclose(arch->file);

		free(arch);
	}
}

/* Similair to fgets(), but doesn't terminate on \n or \0 */
void __fgetstrn(char *dest, int length, FILE* file)
{
	for (int i = 0; i < length; i++)
	{
		dest[i] = fgetc(file);
	}
}
