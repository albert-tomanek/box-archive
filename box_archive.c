#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "dbg.h"
#include "ezxml/ezxml.h"

#include "box_archive.h"
#include "entrylist.h"
#include "entry.h"
#include "filesystem.h"
#include "byteorder.h"
#include "dupcat.h"
#include "positions.h"
#include "ints.h"
#include "types.h"

typedef struct ezxml ezxml;		/* Not implemented in EzXML for some reason... */

/* Private stuff */

void      __ba_load_dir_tree	  (char *orig_header, ba_Entry **first_entry);
void      __ba_process_xml_dir	  (ezxml *parent_xml, ba_Entry **first_entry, ba_Entry *parent_dir);	/* parent_dir=NULL if toplevel dir */
ba_Entry* __ba_get_file_metadata  (ezxml *file_node, ba_Entry *parent_dir);
ba_Entry* __ba_get_dir_metadata	  (ezxml *dir_node, ba_Entry *parent_dir);
ba_Entry* __ba_get_rec_func		  (ba_Entry *first_entry, char *path);
void      __ba_dir_entry_to_xml	  (ezxml *parent_node, ba_Entry *first_entry);
void      __ba_create_header	  (BoxArchive *arch);
void      __ba_load_file_data     (BoxArchive *arch);
void      __ba_load_dir_file_data (ba_Entry *first_entry, fsize_t *total_size);		/* Sorry for the confusing name -- someone change it, if you come up with a more descriptive one... */
int       __ba_create_archive_file(BoxArchive *arch, char *loc);	/* Returns 0 on sucess, and >0 on failure */
void      __ba_save_entry_dir(ba_Entry *first_entry, FILE *outfile);

char*     __ba_load_header (FILE* file);
hdrlen_t  __ba_get_hdrlen  (FILE* file);	/* Used only by ba_load(), and not by ba_save().	*/

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
	arch->__data_size= 0;

	return arch;

error:
	return NULL;
}

void ba_save(BoxArchive *arch, char *loc)
{
	check(arch, "Null-pointer given to ba_save().");

	__ba_load_file_data(arch);	/* Creates a ->file struct for each entry, but doesn't load any files into memory */

	__ba_create_header(arch);	/* Creates an XML header of the archive's structure. */

	__ba_create_archive_file(arch, loc);	/* Actually writes the archive to a file. */

	return;

error:
	return;
}

int __ba_create_archive_file(BoxArchive *arch, char *loc)
{
	/* This function creates a BOX file at the given location	*
	 * from the given BoxArchive struct.						*/

	FILE* outfile = NULL;
	outfile = fopen(loc, "w");		/* Overwrite the current file if it exists */

	check(outfile != NULL, "Error opening output file \"%s\".", loc);

	const uint8_t ba_magic_number[] = {0xDE, 0xCA, 0xDE, BA_FMT_VER};	/* (The last byte is a macro) */
	const uint8_t end_of_header[]   = {0x00, 0xFF};
	const uint8_t end_of_file[]     = {0x00, 0xFF};

	/* Write the header to the file */
	fwrite(ba_magic_number, sizeof(ba_magic_number), 1, outfile);		/* First write the non-changing magic number, and format version. */

	fputc(0x00, outfile);	/* Padding */

	uint64_t header_length = strlen(arch->header);		/* For hdrlen_t see types.h */
	check(header_length < 65534, "XML header too large to fit into v1 of the BoxArchive format. :'-(");
	uint16_t header_length16 = stol16((uint16_t) header_length);
	fwrite(&header_length16, sizeof(hdrlen_t), 1, outfile);	/* Write the length of the XML header as uint16_t to the file. */

	fwrite(arch->header, sizeof(char), strlen(arch->header), outfile);	/* Write the actual XML header to the file. */

	fwrite(end_of_header, sizeof(end_of_header), 1, outfile);

	__ba_save_entry_dir(arch->entry_list, outfile);

	fwrite(end_of_file, sizeof(end_of_file), 1, outfile);

	fclose(outfile);

	return 0;

error:
	if (outfile)	fclose(outfile);

	return 1;
}

void __ba_save_entry_dir(ba_Entry *first_entry, FILE *outfile)
{
	check(first_entry != NULL, "Null-pointer given to __ba_save_entry_dir() for ba_Entry *first_entry.");
	check(outfile     != NULL, "Null-pointer given to __ba_save_entry_dir() for FILE *outfile.");
	/* TODO: Give an error of the file is not open. */

	ba_Entry *current_entry = first_entry;
	FILE     *current_file  = NULL;

	while (current_entry)
	{
		if (current_entry->type == ba_EntryType_FILE)
		{
			current_file = fopen(current_entry->__orig_loc, "r");

			if (current_file == NULL)
			{
				log_err("Error opening file \"%s\" (at \"%s\" in archive).", current_entry->__orig_loc, current_entry->path);
				goto error;
			}

			for (int byte = fgetc(current_file); byte != EOF; byte = fgetc(current_file))
			{
				fputc(byte, outfile);
			}

			fclose(current_file);

			current_file  = NULL;
		}
		else if (current_entry->type == ba_EntryType_DIR)
		{
			__ba_save_entry_dir(current_entry->child_entries, outfile);
		}

		current_entry = current_entry->next;
	}

	return;

error:
	return;
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

void ba_add(ba_Entry **parent_entry, ba_Entry *add_entry)
{
	/* Note that this function expects that add_entry 	*
	 * is correctly filled in, otherwise there will be	*
	 * errors at later stages.							*/

	check(parent_entry, "Null pointer given for ba_Entry **parent_entry to ba_add().");
	check(add_entry   , "Null pointer given for ba_Entry *add_entry to ba_add().");

	bael_add(&((*parent_entry)->child_entries), add_entry);	/* Sorry about the messy pointer de-referencing...	*/

error:
	return;
}

void ba_add_file(ba_Entry **parent_entry, char *file_name, char *loc)
{
	check(parent_entry, "Null pointer given for ba_Entry **parent_entry to ba_add_file().");

	ba_Entry *add_entry = malloc(sizeof(ba_Entry));

	add_entry->type       = ba_EntryType_FILE;
	add_entry->__orig_loc = strdup(loc);
	add_entry->path       = dupcat((*parent_entry)->path ? (*parent_entry)->path : "", file_name, "", "");
	add_entry->name       = strdup(file_name);
	add_entry->file_data  = NULL;
	add_entry->parent_dir = *parent_entry;	/* Doesn't matter if it's NULL */
	add_entry->child_entries = NULL;

	bael_add(&((*parent_entry)->child_entries), add_entry);

error:
	return;
}

void ba_add_dir(ba_Entry **parent_entry, char *dir_name)
{
	check(parent_entry, "Null pointer given for ba_Entry **parent_entry to ba_add_dir().");

	ba_Entry *add_entry = malloc(sizeof(ba_Entry));

	add_entry->type       = ba_EntryType_DIR;
	add_entry->__orig_loc = NULL;
	add_entry->path       = dupcat((*parent_entry)->path ? (*parent_entry)->path : "", dir_name, (dir_name[strlen(dir_name)-1] == BA_SEP[0] ? "" : BA_SEP), "");
	add_entry->name       = strdup(dir_name);
	add_entry->file_data  = NULL;
	add_entry->parent_dir = *parent_entry;	/* Doesn't matter if it's NULL */
	add_entry->child_entries = NULL;		/* For now... */

	bael_add(&((*parent_entry)->child_entries), add_entry);

error:
	return;
}

void __ba_load_file_data(BoxArchive *arch)
{
	/* Goes through an archive's entry tree, 		*
	 * and creates a ba_File struct for each entry,	*
	 * containing the file's size, etc.				*/

	check(arch, "Null-pointer given to __ba_load_file_data().");

	__ba_load_dir_file_data(arch->entry_list, &(arch->__data_size) );

	return;

error:
	return;
}

void __ba_load_dir_file_data(ba_Entry *first_entry, fsize_t *total_size)
{
	/* Recursive function used by __ba_load_file_data()	*/

	check(total_size, "Null-pointer given to __ba_load_dir_file_data() for int *total_size.");

	if (! first_entry)
	{
		/* In case of an empty directory. */
		return;
	}

	ba_Entry *current = first_entry;

	while (current)
	{
		if (current->type == ba_EntryType_FILE)
		{
			if (! current->__orig_loc)
			{
				/* ->__orig_loc is NEEDED so that ba knows 	*
				 * where to read the source file from when 	*
				 * creating the archive.					*/

				log_warn("Source file for \"%s\" unspecified. Developer: please call ba_load_fs_tree() or ba_add() before calling ba_save().", current->path);		/* May ba a bit vague to a clueless developer; please improve if you can come up with a better error message... */
				goto error;
			}

			current->file_data = calloc(1, sizeof(ba_File));
			current->file_data->__size  = ba_fsize(current->__orig_loc);
			current->file_data->__start = *total_size;

			*total_size += current->file_data->__size;	/* We're not incrementing the pointer; we're incrementeng the actual integer that is pointed to. */
		}
		else if (current->type == ba_EntryType_DIR)
		{
			/* Directory entries' ->file_data	*
			 * is a null-pointer.				*/

			__ba_load_dir_file_data(current->child_entries, total_size);
		}

		current = current->next;
	}

	return;

error:
	return;
}

void __ba_create_header(BoxArchive *arch)
{
	/* This function recursiveley goes through an archive's	*
	 * entry tree, and creates an XML representation of it,	*
	 * which arch->header points to.						*/

	check(arch, "Null-pointer given to __ba_create_header().");

	ezxml *xml  = ezxml_new("header");
	check(xml, "Error creating XML header: %s", ezxml_error(xml));

	__ba_dir_entry_to_xml(xml, arch->entry_list);

	if (arch->header)	free(arch->header);		/* Frees the old header if one exists */
	arch->header = 	ezxml_toxml(xml);

	return;

error:
	return;
}

void __ba_dir_entry_to_xml(ezxml *parent_node, ba_Entry *first_entry)
{
	/* Recursive function used by __ba_create_header().	*/

	check(parent_node, "Null-pointer given to __ba_dir_entry_to_xml() for ezxml *parent_node.");
	check(parent_node, "Null-pointer given to __ba_dir_entry_to_xml() for ba_Entry *first_entry.");

	ezxml	 *current_node = NULL;
	ba_Entry *current      = first_entry;

	char size_attr_str [BA_INTLEN];
	char start_attr_str[BA_INTLEN];

	while (current)
	{
		current_node = ezxml_add_child(parent_node, ba_entry_xml_type(current->type), 0);
		ezxml_set_attr(current_node, "name", current->name);

		if (current->type == ba_EntryType_FILE)
		{
			if (current->file_data)
			{
				snprintf(size_attr_str,  BA_INTLEN, "%lld", (long long unsigned) current->file_data->__size);
				snprintf(start_attr_str, BA_INTLEN, "%lld", (long long unsigned) current->file_data->__start);
				ezxml_set_attr_d(current_node, "size",  size_attr_str );	/* _d calls a wrapper for ezxml_set_attr() that strdup()'s the strings passed to it. */
				ezxml_set_attr_d(current_node, "start", start_attr_str );
			}
		}
		else if (current->type == ba_EntryType_DIR)
		{
			__ba_dir_entry_to_xml(current_node, current->child_entries);
		}

		current = current->next;
	}

	return;

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

ba_Entry* ba_get(BoxArchive *arch, char *orig_path)
{
	check(arch, "Null pointer given for BoxArchive *arch to ba_get().");

	char *path = NULL;		/* Sorry, could be cleaner... */
	path = dupcat(orig_path, (orig_path[strlen(orig_path)-1] == BA_SEP[0] ? "" : BA_SEP), "", "");		/* This appends a '/' to the end of the path, because ba_Entry->name/path _always_ ends with a '/' if the entry is a directory.	*/
	ba_Entry *return_entry = __ba_get_rec_func(arch->entry_list, path);

	free(path);
	return    return_entry;

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
