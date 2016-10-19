#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "box_archive.h"
#include "filelist.h"
#include "errors.h"
#include "positions.h"
#include "ints.h"
#include "ezxml/ezxml.h"

typedef struct ezxml ezxml;		/* Not implemented in EzXML for some reason... */

/* Private stuff */

void  __ba_dir_process(ezxml *parent, ba_FileList **first_file, char *current_dir);
void  __ba_cleanup(BoxArchive *arch);
char* __dupcat(char *str1, char *str2, char *str3);
void  __fgetstrn(char *dest, int length, FILE* file);

/* Library functions */

BoxArchive* ba_open(char *loc)
{
	BoxArchive *arch = malloc(sizeof(BoxArchive));

	if (!arch) {
		error(0, "[ERROR] Out of memory.\n");
		return NULL;
	}
	
	/* Constructor */
	arch->__debug = 0;
	
	/* Open the actual file */
	arch->file = fopen(loc, "rw");

	if (! arch->file) {
		error(0, "[ERROR] The file could not be opened.\n");
		return NULL;
	}
	
	/* Set arch->loc */
	arch->loc = strdup(loc);
	
	uint8_t format = ba_get_format(arch);

	if (format == 0)
	{
		error(ERR_FFORMAT, "[ERROR] Not a box archive.\n");
		return NULL;
	}
	else if (format > BA_MAX_VER)
	{
		error(ERR_FFORMAT, "[ERROR] Format version %d not supported.", format);
	}

	return arch;
}

void ba_list(BoxArchive *arch)
{

}

ba_FileList* ba_get_files(BoxArchive *arch)
{
	/* The XML header extracted from the archive */
	char *header = ba_gethdr(arch);
	
	/* The (yet to be initialised) list we'll be returning */
	ba_FileList *first_file = NULL;
	
	/* The root directory of the archive (ie. root node of the XML) */
	ezxml *root_dir = ezxml_parse_str(header, strlen(header));
	
	if (!root_dir)
	{
		error(0, "[ERROR] XML parser error: %s\n", ezxml_error(root_dir));
		return NULL;
	}
	
	__ba_dir_process(root_dir, &first_file, "");
	
	ezxml_free(root_dir);
	free(header);
	
	return first_file;
}

void __ba_dir_process(ezxml *parent, ba_FileList **first_file, char *current_dir)
{
	/* Recursive function used by ba_get_files(),
	 * and add each file to the given ba_FileList.
	 */
	
	ezxml *file = ezxml_child(parent, "file");	/* A linked list of all the file nodes;			*/
	ezxml *dir  = ezxml_child(parent, "dir");	/* A linked list of all the directory nodes.	*/
	
	const char *file_name;			/* 'const' else it gives a warning for some reason...	*/
	const char *dir_name;
	char       *joint_file_name;
	char       *child_dir_name;
	
	while (file)
	{
		file_name = ezxml_attr(file, "name");
		
		joint_file_name = __dupcat(current_dir, (char*) file_name, "");
		
		/* Add the file path to the list */
		bafl_add(first_file, joint_file_name);
		
		free(joint_file_name);
		file = file->next;
	}
	
	while (dir)
	{
		dir_name   = ezxml_attr(dir, "name");
		
		/* make a string with the directory name to pass to the recursing function */
		child_dir_name = __dupcat(current_dir, (char*) dir_name, BA_SEP);
		
		__ba_dir_process(dir, first_file, child_dir_name);
		
		free(child_dir_name);
		dir = dir->next;
	}
}

void ba_debug(BoxArchive *arch, uint8_t debug)
{
	if (! arch)
	{
		error(0, "[ERROR] Null-pointer given to ba_debug().\n");
	}
	else
	{
		if (! arch->file)
		{
			error(0, "[ERROR] File not open!\n");
		}
	}
	
	arch->__debug = debug ? 1 : 0 ;
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
		error(0, "[ERROR] Null-pointer given to ba_get_hdrlen().\n");
		return 0;
	}
	if (! arch->file)
	{
		error(0, "[ERROR] File not open!\n");
		return 0;
	}
	
	/* Go to the two-byte header length */
	fseek(arch->file, P_HEADER_LENGTH, SEEK_SET);
	
	fgetc(arch->file);
	uint8_t byte1	=	fgetc(arch->file);
	uint8_t byte2	=	fgetc(arch->file);
	
	return cvt8to16(byte1, byte2);	/* function from ints.h */
}

char* ba_get_header(BoxArchive *arch)
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
	
	/* Get the header length and allocate enough memory to copy it in to */
	uint16_t hdr_length = ba_get_hdrlen(arch);
	
	if (arch->__debug)
		printf("[DEBUG] XML header length = %d bytes\n", hdr_length);
		
	char* header = calloc(hdr_length+1, sizeof(char));	/* +1 for the null-byte */
	
	if (! header)
	{
		error(ERR_MEM, "[ERROR] Out of memory (malloc() returned NULL).\n");
	}
	
	/* Copy the header from the file into memory */
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
		
		if (arch->loc)
			free(arch->loc);
		
		free(arch);
	}
}

void __ba_cleanup(BoxArchive *arch)
{
	if (! arch)
	{
		error(0, "[ERROR] Null-pointer given to __ba_cleanup().\n");
		return;
	}
	
	/* Free all elments on heap */
	if (arch->loc)
		free(arch->loc);
	
	if (arch->file)
		fclose(arch->file);
	else
		fprintf(stderr, "[WARNING] Could not close archive; archive not open.\n");
	
	free(arch);
}

/* Similair to fgets(), but doesn't terminate on \n or \0 */
void __fgetstrn(char *dest, int length, FILE* file)
{
	for (int i = 0; i < length; i++)
	{
		dest[i] = fgetc(file);
	}
}

char* __dupcat(char *str1, char *str2, char *str3)
{
	/* Like strcat, but does strdup first */
	char *out;
	
	out = calloc( strlen(str1)+strlen(str2)+strlen(str3)+1, sizeof(char));
	sprintf(out, "%s%s%s", str1, str2, str3);
	
	return out;
}
