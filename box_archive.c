#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "box_archive.h"
#include "entrylist.h"
#include "entry.h"
#include "positions.h"
#include "ints.h"
#include "types.h"
#include "dbg.h"
#include "ezxml/ezxml.h"

typedef struct ezxml ezxml;		/* Not implemented in EzXML for some reason... */

/* Private stuff */

void     __ba_process_xml_dir(ezxml *parent, ba_EntryList **first_entry, char *current_dir);
ba_Entry* __ba_get_file_metadata(ezxml *file, char *current_dir);
ba_Entry* __ba_get_dir_metadata (ezxml *dir, char *current_dir);

ba_EntryList* __ba_load_metadata(char *header);

char*     __ba_load_header(FILE* file);
hdrlen_t  __ba_get_hdrlen(FILE* file);
void      __ba_cleanup(BoxArchive *arch);

char* __dupcat(char *str1, char *str2, char *str3);
void  __fgetstrn(char *dest, int length, FILE* file);

/* Library functions */

BoxArchive* ba_open(char *loc)
{
	BoxArchive *arch = malloc(sizeof(BoxArchive));

	check(arch, "Out of memory!");

	/* Open the actual file */
	arch->file = fopen(loc, "rw");

	check(arch->file, "The file could not be opened.");

	/* Set arch->loc */
	arch->loc = strdup(loc);

	/* Check the format version */
	uint8_t format = ba_get_format(arch);

	check(format != 0, "Not a box archive.");
	check(format <= BA_MAX_VER, "Format version %d not supported.", format);

	/* Copy the XML header into memory */
	arch->header = __ba_load_header(arch->file);

	/* Read the metadata from the header */
	arch->entry_list = __ba_load_metadata(arch->header);

	return arch;

error:

	if (arch)
	{
		if (arch->file)
			fclose(arch->file);
		free(arch);
	}

	return NULL;
}

ba_EntryList* ba_get_entries(BoxArchive *arch)
{
	check(arch, "Null pointer given to ba_get_entrys().")

	return arch->entry_list;

error:
	return NULL;
}

ba_EntryList* __ba_load_metadata(char *orig_header)
{
	/* This function gets the XML header,			*
	 * and makes a ba_EntryList from it.			*/

	check(orig_header, "Null-pointer passed to __ba_load_metadata().");

	/* The (yet to be initialised) list we'll be returning */
	ba_EntryList *first_entry = NULL;
	char *header = strdup(orig_header);		/* We need our own copy, because I think ezxml calls strtok on it */

	/* The root directory of the archive (ie. root node of the XML) */
	ezxml *root_dir = ezxml_parse_str(header, strlen(header));

	check(root_dir, "XML parser error: %s", ezxml_error(root_dir));

	__ba_process_xml_dir(root_dir, &first_entry, "");		/* Run the recursive (self-calling) function that goes through all the directories and adds everything to the list */

	ezxml_free(root_dir);
	free(header);

	return first_entry;

error:
	return NULL;
}

void __ba_process_xml_dir(ezxml *parent, ba_EntryList **first_entry, char *current_dir)
{
	/* Recursive function used by __ba_load_metadata(),
	 * to add each entry to the given ba_EntryList.
	 */

	ezxml *file_node = ezxml_child(parent, "file");	/* A linked list of all the file nodes;			*/
	ezxml *dir_node  = ezxml_child(parent, "dir");	/* A linked list of all the directory nodes.	*/

	ba_Entry   *entry;

	while (file_node)
	{
		entry = __ba_get_file_metadata(file_node, current_dir);

		/* Add the file path to the list */
		bael_add(first_entry, entry);

		file_node = file_node->next;
	}

	while (dir_node)
	{
		/* Get the directory's metadata */
		entry = __ba_get_dir_metadata(dir_node, current_dir);

		/* Add the directory to the entry list */
		bael_add(first_entry, entry);

		/* make a string with the directory name to pass to the recursing function */
		__ba_process_xml_dir(dir_node, first_entry, entry->path);

		dir_node = dir_node->next;
	}
}

ba_Entry* __ba_get_file_metadata(ezxml *file_node, char *current_dir)
{
	/* Reads a file's metadata from the given XML node,	*
	 * and puts it into a struct.						*/

	char *joint_file_name;
	joint_file_name = __dupcat(current_dir, (const char*) ezxml_attr(file_node, "name"), "");

	ba_Entry *file = malloc(sizeof(ba_Entry));

	check(file, "malloc returned NULL.");

	file->name = strdup( (const char*) ezxml_attr(file_node, "name") );
	file->path = joint_file_name;		/* Note: the string will be freed when the struct is freed.	*/
	file->type = ba_EntryType_FILE;

	file->__size  = atoi( ezxml_attr(file_node, "size")  );
	file->__start = atoi( ezxml_attr(file_node, "start") );

	return file;

error:
	return NULL;
}

ba_Entry* __ba_get_dir_metadata(ezxml *dir_node, char *current_dir)
{
	/* Reads a file's metadata from the given XML node,	*
	 * and puts it into a struct.						*/

	char *joint_dir_name;
	joint_dir_name = __dupcat(current_dir, (const char*) ezxml_attr(dir_node, "name"), BA_SEP);

	ba_Entry *dir = malloc(sizeof(ba_Entry));

	check(dir, "malloc returned NULL.");

	dir->name = strdup( (const char*) ezxml_attr(dir_node, "name") );
	dir->path = joint_dir_name;		/* Note: the string will be freed when the struct is freed.	*/
	dir->type = ba_EntryType_DIR;

	return dir;

error:
	return NULL;
}

int ba_extract(BoxArchive *arch, char *path, char *dest)
{
	/* What do you *think* an extract function would do? */

	/* Get the struct with the metadata of the file */
	ba_Entry* file_meta = bael_get(arch->entry_list, path);

	check(file_meta, "File at '%s' not found.", path);

	/* Go to the start of the file in the data chunk */
	fseek(arch->file, P_FILE_DATA + strlen(arch->header) + file_meta->__start, SEEK_SET);

	/* Open the file to write to */
	FILE* out_file = fopen(dest, "w+");

	for (fsize_t offset = 0; offset < file_meta->__size; offset++)
	{
		/* Write each byte out to the file */
		fputc(fgetc(arch->file), out_file);
	}

	return 0;

error:
	return 1;
}

uint8_t ba_get_format(BoxArchive *arch)
{
	/* If the file is not a BOX archive, *
	 * 0 will be returned				 */

	check(arch,       "Null-pointer given to ba_get_format().");
	check(arch->file, "Entry not open!");

	rewind(arch->file);				/* Go to the start of the file */

	uint8_t hdr_bytes[4];

	hdr_bytes[0] = fgetc(arch->file);
	hdr_bytes[1] = fgetc(arch->file);
	hdr_bytes[2] = fgetc(arch->file);
	hdr_bytes[3] = fgetc(arch->file);

	if (hdr_bytes[0] != 0xde || hdr_bytes[1] != 0xca || hdr_bytes[2] != 0xde) {	/* Pure genious */
		return 0;
	} else {
		return hdr_bytes[3];
	}

error:
	return 0;
}

hdrlen_t __ba_get_hdrlen(FILE* file)
{
	check(file, "Entry not open!");

	/* Go to the two-byte header length field */
	fseek(file, P_HEADER_LENGTH, SEEK_SET);

	fgetc(file);
	uint8_t byte1	=	fgetc(file);
	uint8_t byte2	=	fgetc(file);

	hdrlen_t length = cvt8to16(byte1, byte2);	/* function from ints.h */

	debug("HDRLEN bytes: %02X %02X", byte1, byte2);
	debug("XML header length = %d bytes", length);

	return length;

error:
	return 0;
}

char* ba_get_header(BoxArchive *arch)
{
	check(arch, "[ERROR] Null-pointer given to ba_get_header().");

	return strdup(arch->header);

error:
	return NULL;
}

char* __ba_load_header(FILE* file)
{
	check(file, "[ERROR] Null-pointer given to __ba_load_header().");

	/* Get the header length and allocate enough memory to copy it in to */
	hdrlen_t hdr_length = __ba_get_hdrlen(file);

	/* Go to the header */
	fseek(file, P_HEADER, SEEK_SET);

	debug("XML header starts at offset %d from start of file.", P_HEADER);

	char* header = calloc(hdr_length+1, sizeof(char));	/* +1 for the null-byte */

	check(header, "Out of memory (malloc() returned NULL).\n");

	/* Copy the header from the file into memory */
	__fgetstrn(header, hdr_length, file);

	return header;

error:
	return NULL;
}

void ba_close(BoxArchive *arch)
{
	if (! arch)
	{
		log_err("Null-pointer given to ba_close().\n");
	}
	else
	{
		if (! arch->file)
			log_warn("Cannot close archive; archive not open!\n");
		else
			fclose(arch->file);

		if (arch->loc)
			free(arch->loc);

		free(arch);
	}
}

void __ba_cleanup(BoxArchive *arch)
{
	check(arch, "Null-pointer given to __ba_cleanup().");
error:

	/* Free all elments on heap */
	if (arch->loc)
		free(arch->loc);
	if (arch->header)
		free(arch->header);

	if (arch->file)
	{
		fclose(arch->file);
	}
	else
	{
		log_warn("Could not close archive; archive not open.");
	}

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
	/* Like strcat, but makes a copy of the text */
	char *out;

	out = calloc( strlen(str1)+strlen(str2)+strlen(str3)+1, sizeof(char));
	sprintf(out, "%s%s%s", str1, str2, str3);

	return out;
}
