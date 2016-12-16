#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "dbg.h"
#include "ezxml/ezxml.h"

#include "box_archive.h"
#include "entrylist.h"
#include "entry.h"
#include "dupcat.h"
#include "positions.h"
#include "ints.h"
#include "types.h"

typedef struct ezxml ezxml;		/* Not implemented in EzXML for some reason... */

/* Private stuff */

void      __ba_load_dir_tree(char *orig_header, ba_Entry **first_entry);
void      __ba_process_xml_dir(ezxml *parent_xml, ba_Entry **first_entry, ba_Entry *parent_dir);	/* parent_dir=NULL if toplevel dir */
ba_Entry* __ba_get_file_metadata(ezxml *file_node, ba_Entry *parent_dir);
ba_Entry* __ba_get_dir_metadata(ezxml *dir_node, ba_Entry *parent_dir);
ba_Entry* __ba_get_rec_func(ba_Entry *first_entry, char *path);

char*     __ba_load_header(FILE* file);
hdrlen_t  __ba_get_hdrlen(FILE* file);

void  __fgetstrn(char *dest, int length, FILE* file);

/* Library functions */

BoxArchive* ba_new()
{
	/* Creates a new, empty BoxArchive struct,	*
	 * and returns a pointer to it.				*/

	BoxArchive *arch = malloc(sizeof(BoxArchive));
	check(arch, "Out of memory!");

	arch->loc        = NULL;		/* Hasn't been saved anywhere yet */
	arch->header     = NULL;
	arch->file       = NULL;
	arch->entry_list = NULL;

	return arch;

error:
	return NULL;
}

BoxArchive* ba_open(char *loc)
{
	/* Reads all the data (except the file data itsself)	*
	 * from a .box file, and returns a pointer to it.		*/

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
	__ba_load_dir_tree(arch->header, &arch->entry_list);

	return arch;

error:

	ba_close(arch);

	return NULL;
}

void ba_add(BoxArchive *arch, ba_Entry *entry)
{
	check(arch, "Null pointer given for BoxArchive *arch to ba_add().");
	check(arch, "Null pointer given for ba_Entry *entry to ba_add().");

	/* YET TO BE IMPLEMENTED */

error:
	return;
}

ba_Entry* ba_get_entries(BoxArchive *arch)
{
	check(arch, "Null pointer given to ba_get_entries().");

	return arch->entry_list;	/* This is the real thing, not a copy, so be careful with it */

error:
	return NULL;
}

ba_Entry* ba_get(BoxArchive *arch, char *path)
{
	check(arch, "Null pointer given for BoxArchive *arch to ba_get().");

	return __ba_get_rec_func(arch->entry_list, path);

error:
	return NULL;
}

ba_Entry* __ba_get_rec_func(ba_Entry *first_entry, char *path)
{
	if (! first_entry)	return NULL;

	ba_Entry *current = first_entry;

	while (current)
	{
		if (! strcmp(current->path, path))		/* This works whether it's a file, or a direcotry */
		{
			return current;
		}

		if (current->type == ba_EntryType_DIR)
		{
			ba_Entry *subdir = __ba_get_rec_func(current->child_entries, path);

			if (subdir != NULL)	return subdir;
		}

		current = current->next;
	}

	return NULL;
}

void __ba_load_dir_tree(char *orig_header, ba_Entry **first_entry)
{
	/* This function reads the XML header,			*
	 * and loads the files' metadata into a tree	*
	 * in memory. It then changes the first_entry	*
	 * to point to this tree.						*/

	check(orig_header, "Null-pointer passed to __ba_load_dir_tree().");

	char *header = strdup(orig_header);		/* We need our own copy, because I think ezxml calls strtok on it */

	/* The root directory of the archive (ie. root node of the XML) */
	ezxml *root_dir = ezxml_parse_str(header, strlen(header));

	check(root_dir, "XML parser error: %s", ezxml_error(root_dir));

	__ba_process_xml_dir(root_dir, first_entry, NULL);		/* Run the recursive (self-calling) function that goes through all the directories and adds everything to the list */

	ezxml_free(root_dir);
	free(header);

	return;

error:
	return;
}

void __ba_process_xml_dir(ezxml *parent_xml, ba_Entry **first_entry, ba_Entry *parent_dir)
{
	/* Recursive function used by __ba_load_dir_tree()	*
	 * Adds all the files/dirs in the given directory	*
	 * to the directory tree which is in memory,		*
	 * which is pointed to by first_entry.				*/

	ezxml *file_node = ezxml_child(parent_xml, "file");	/* A linked list of all the file nodes;			*/
	ezxml *dir_node  = ezxml_child(parent_xml, "dir");	/* A linked list of all the directory nodes.	*/

	ba_Entry   *entry;

	while (file_node)
	{
		entry = __ba_get_file_metadata(file_node, parent_dir);

		/* Add the file path to the list */
		bael_add( parent_dir ? &(parent_dir->child_entries) : first_entry, entry);		/* If we are in the top level directory and parent_dir is NULL,	*
																				 		 * the pointer to parent_dir->child_entries would be false.		*/

		file_node = file_node->next;
	}

	while (dir_node)
	{
		/* Get the directory's metadata */
		entry = __ba_get_dir_metadata(dir_node, parent_dir);

		/* Add the directory to the entry list */
		bael_add( parent_dir ? &(parent_dir->child_entries) : first_entry, entry);

		/* Run this function again, this time on the newly loaded directory */
		__ba_process_xml_dir(dir_node, &(entry->child_entries), entry);

		dir_node = dir_node->next;
	}
}

ba_Entry* __ba_get_file_metadata(ezxml *file_node, ba_Entry *parent_dir)
{
	/* Reads a file's metadata from the given XML node,	*
	 * and puts it into a ba_Entry and ba_File struct.	*/

	char *joint_file_name;
	joint_file_name = dupcat( parent_dir ? parent_dir->path : "" , (const char*) ezxml_attr(file_node, "name"), "", "");

	ba_Entry *file_entry = malloc(sizeof(ba_Entry));	/* The entry used to hold the file's metadata */
	ba_File  *file_data  = malloc(sizeof(ba_File));		/* The struct used to hold the file's position in the box archive, and a buffer of the contents */
	check(file_entry && file_data, "malloc returned NULL.");

	file_entry->file_data = file_data;
	file_entry->parent_dir = parent_dir;
	file_entry->child_entries = NULL;

	file_entry->name = strdup( (const char*) ezxml_attr(file_node, "name") );
	file_entry->path = joint_file_name;		/* Note: the string will be freed when the struct is freed.	*/
	file_entry->type = ba_EntryType_FILE;

	file_data->contents = NULL;						/* The file will only be loaded into memory if it is being modified */
	file_data->__size   = atoi( ezxml_attr(file_node, "size")  );
	file_data->__start  = atoi( ezxml_attr(file_node, "start") );

	return file_entry;

error:
	return NULL;
}

ba_Entry* __ba_get_dir_metadata(ezxml *dir_node, ba_Entry *parent_dir)
{
	/* Reads a directory's metadata from the given XML node,	*
	 * and puts it into a ba_Entry struct.						*/

	char *joint_dir_name;
	joint_dir_name = dupcat( parent_dir ? parent_dir->path : "", (const char*) ezxml_attr(dir_node, "name"), BA_SEP, "");

	ba_Entry *dir = malloc(sizeof(ba_Entry));

	check(dir, "malloc returned NULL.");

	dir->type = ba_EntryType_DIR;
	dir->name = strdup( (const char*) ezxml_attr(dir_node, "name") );
	dir->path = joint_dir_name;		/* Note: the string will be freed when the struct is freed.	*/

	dir->file_data  = NULL; 		/* This is not a file */
	dir->parent_dir = parent_dir;
	dir->child_entries = NULL;		/* For *now*. This will ba changed by calling __ba_get_file_metadata() on the direcotry. */

	dir->next = NULL;	/* Will be changed once another file/dir is loaded in the same parent dir */

	return dir;

error:
	return NULL;
}

int ba_extract(BoxArchive *arch, ba_Entry *file_entry, char *dest)
{
	check(file_entry, "Null-pointer given to ba_extract().");

	fsize_t file_size = file_entry->file_data->__size;

	/* Go to the start of the file in the data chunk */
	fseek(arch->file, P_FILE_DATA + strlen(arch->header) + file_entry->file_data->__start, SEEK_SET);

	/* Open the file to write to */
	FILE* out_file = fopen(dest, "w+");
	check(out_file, "File at \"%s\" could not be opened for writing.", dest);

	for (fsize_t offset = 0; offset < file_size; offset++)
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
	 * 0 will be returned.				 */

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
	/* Returns the length (in bytes)	*
	 * of the XML header.				*/

	check(file, "Entry not open!");

	/* Go to the two-byte header length field */
	fseek(file, P_HEADER_LENGTH, SEEK_SET);

	fgetc(file);
	uint8_t byte1	=	fgetc(file);
	uint8_t byte2	=	fgetc(file);

	hdrlen_t length = cvt8to16(byte1, byte2);	/* function from ints.h */

	return length;

error:
	return 0;
}

char* ba_get_header(BoxArchive *arch)
{
	/* Returns a copy of the 	*
	 * archive's XML header.	*/

	check(arch, "[ERROR] Null-pointer given to ba_get_header().");

	return strdup(arch->header);

error:
	return NULL;
}

char* __ba_load_header(FILE* file)
{
	/* Loads the XML header from the 	*
	 * given .box file into memory.		*/

	check(file, "[ERROR] Null-pointer given to __ba_load_header().");

	/* Get the header length and allocate enough memory to copy it in to */
	hdrlen_t hdr_length = __ba_get_hdrlen(file);

	/* Go to the header */
	fseek(file, P_HEADER, SEEK_SET);

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
	/* Frees a BoxArchive struct,	*
	 * and its contents.			*/

	if (! arch)
	{
		log_err("Null-pointer given to ba_close().\n");
	}
	else
	{
		/* Free strings */
		if (arch->loc)		free(arch->loc);
		if (arch->header)	free(arch->header);

		/* Call other free functions */
		if (arch->file)			fclose(arch->file);
		if (arch->entry_list)	bael_free(&arch->entry_list);

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
