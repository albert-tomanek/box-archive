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
#include "types.h"

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
	arch->entry_tree = NULL;
	arch->__data_size= 0;

	return arch;

error:
	return NULL;
}

void ba_save(BoxArchive *arch, char *loc)
{
	check(arch, "Null-pointer given to ba_save().");

	if (arch->loc != NULL)
	{
		if (! strcmp(arch->loc, loc))
		{
			/* If we are overwriting the original file,	load all the file data 	*
			 * from the original file into buffer.								*
			 * Otherwise we wouldn't have a source for them.					*/

			__ba_buffer_entries(arch);
		}
	}

	__ba_rearrange(arch);

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

	arch->__data_size = 0;		/* I know that __ba_rearrange() already does this, sorry! */

	__ba_save_entry_dir(arch->entry_tree, arch->file, outfile, &(arch->__data_size));	/* Note: arch->file has not been changed yet, so it will still be the source archive. */

	fwrite(end_of_file, sizeof(end_of_file), 1, outfile);

	/* Update the ->loc and ->file members of the archive */

	if (arch->loc)	free(arch->loc);
	arch->loc  = strdup(loc);

	if (arch->file)	fclose(arch->file);
	arch->file = outfile;

	return 0;

error:
	if (outfile)	fclose(outfile);

	return 1;
}

void __ba_save_entry_dir(ba_Entry *first_entry, FILE *infile, FILE *outfile, fsize_t *total_size)
{
	/* This procedure is used by ba_save(), 		*
	 * and goes through an entry tree writing the 	*
	 * contents of each file entry to 'outfile'.	*
	 * The contents are either read from the file 	*
	 * at entry->__orig_loc, or from the buffer at 	*
	 * entry->file_data->buffer. (If neither of 	*
	 * these exist, the file is a zero-byte file.)	*
	 *												*
	 * This results in all the files' contents 		*
	 * joined end-to-end in 'outfile'.				*/

	check(outfile != NULL, "Null-pointer given to __ba_save_entry_dir() for FILE *outfile.");
	/* TODO: Give an error if the file is not open. */

	ba_Entry *current_entry = first_entry;		/* If this is null, the directory is empty and the loop will be skipped */
	FILE     *current_file  = NULL;

	while (current_entry)
	{
		if (current_entry->type == ba_EntryType_FILE)
		{
			if (current_entry->file_data == NULL)
			{
				/* If there's no file_data */

				log_err("File data for \"%s\" (%s) not present. The archive will be corrupt.", current_entry->path, (current_entry->__orig_loc) ? current_entry->__orig_loc : "already in archive");

				current_entry = current_entry->next;
				continue;
			}

			/* NOTE: The if...elseif...else must be in this order,				*
			 *       because data in the buffer has the highest priority.		*
			 *		 If an entry had both ->__orig_loc and ->file_data->buffer,	*
			 *       then the data in the buffer will most likely be the newest	*/

		 	/* If current_entry->file_data->__start != *total_size,	*
			 * something has gone terribly wrong...					*/

			if (current_entry->file_data->buffer != NULL)
			{
				/* If there is data in the buffer... */

				uint8_t byte = 0;

				for (offset_t offset = 0; offset < current_entry->file_data->__size; offset++)
				{
					byte = current_entry->file_data->buffer[offset];
					fputc(byte, outfile);
				}
			}
			else if (current_entry->__orig_loc != NULL)
			{
				/* If the data is to be read from another file... */

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
			else if (current_entry->file_data->__old_start >= 0)
			{
				/* If the data is to be read from the source archive */

				fseek(infile, (P_FILE_DATA + __ba_get_hdrlen(infile) + current_entry->file_data->__old_start), SEEK_SET);

				int byte;

				for (offset_t i = 0; i < current_entry->file_data->__size; i++)
				{
					byte = fgetc(infile);
					check(byte != EOF, "Unexpected end of archive file.");
					fputc(byte, outfile);
				}

			}
			else if (current_entry->file_data->__size == 0)
			{
				/* If the file is a zero-byte file,	*
				 * there *shouldn't* be a source.	*/
			}
			else
			{
				log_err("No source for \"%s\" found.", current_entry->path);

				current_entry = current_entry->next;
				continue;
			}

			/* Increment the total_size by the current file's size,		*
			 * so that the next file's ->file_data->__start can be set	*/

			*total_size += current_entry->file_data->__size;

		}
		else if (current_entry->type == ba_EntryType_DIR)
		{
			__ba_save_entry_dir(current_entry->child_entries, infile, outfile, total_size);
		}

		current_entry = current_entry->next;
	}

	return;

error:
	return;
}

void __ba_rearrange(BoxArchive *arch)
{
	/* This function resets all file's start positions	*
	 * (saving them in a temporary variable), and		*
	 * gives them new start offsets, starting from 0.	*
	 * 													*
	 * For example, this is necessary when a file has 	*
	 * been removed from the middle of the data block,	*
	 * and there would be a gap in start offsets.		*/

	check(arch, "Null-pointer given to __ba_rearrange()");

	arch->__data_size = 0;

	__ba_rec_rearrange_func(arch->entry_tree, &(arch->__data_size));

	return;

error:
	return;
}

void __ba_rec_rearrange_func(ba_Entry *first_entry, offset_t *data_size)
{
	/* Don't do a null-check for first_entry, because it may be an empty direcotry. */
	check(data_size, "Null-pointer given for *data_size to __ba_rec_rearrange_func()");

	ba_Entry *current = first_entry;

	while (current)
	{
		if (current->type == ba_EntryType_FILE)
		{
			if (! current->file_data)
			{
				log_err("No file data present for '%s'. >:-{", current->path);
			}

			current->file_data->__old_start = current->file_data->__start;
			current->file_data->__start = *data_size;
			*data_size += current->file_data->__size;
		}
		if (current->type == ba_EntryType_DIR)
		{
			__ba_rec_rearrange_func(current->child_entries, data_size);
		}

		current = current->next;
	}

	return;

error:
	return;
}

void __ba_buffer_entries(BoxArchive *arch)
{
	/* This function loads all the entries in the given 				*
	 * tree to their ->file_data->buffer, providing that the file has 	*
	 * not been newly added and is in the existing archive file.		*
	 *																	*
	 * If a file is already in the buffer, it will not be re-read.		*
	 *																	*
	 * If a file is not buffered (ie. newly added with ba_add_file()),	*
	 * the contents of its ->__orig_loc will be copied into the archive *
	 * when __ba_save_entry_dir() is called from ba_save().				*/

	check(arch, "Null-pointer given to __ba_buffer_entries().");

	__ba_buffer_dir(arch, arch->entry_tree);

	return;

error:
	return;
}

void __ba_buffer_dir(BoxArchive *arch, ba_Entry *first_entry)
{
	check(arch, "Null-pointer given to __ba_buffer_dir() for BoxArchive *arch.");

	ba_Entry *current = first_entry;	/* If this is a null-pointer, the dir most likley hasn't got any child entries, and so the loop will be skipped. */

	while (current)
	{
		if (current->type == ba_EntryType_FILE)
		{
			if (! current->file_data)
			{
				/* Shouldn't be needed but it's here just in case */

				log_warn("(__ba_buffer_dir()) File data for '%s' (%s) not present. Skipping.", current->path, (current->__orig_loc) ? current->__orig_loc : "already in archive");
			}

			__ba_buffer_file(arch, current);
		}
		if (current->type == ba_EntryType_DIR)
		{
			__ba_buffer_dir(arch, current->child_entries);
		}

		current = current->next;
	}

	return;

error:
	return;
}

void __ba_buffer_file(BoxArchive *arch, ba_Entry *entry)
{
	/* This function loads a file's data from its archive file	*
	 * into memory. IT DOES NOT LOAD THE FILE IF IT HAS BEEN 	*
	 * NEWLY ADDED AND REFERED TO BY ->__orig_loc.				*/

	check(arch,  "Null-pointer given to __ba_buffer_file() for BoxArchive *arch.");
	check(entry, "Null-pointer given to __ba_buffer_file() for ba_Entry *entry.");

	if (entry->file_data->buffer != NULL)
	{
		/* If the file is already in the buffer,		*
		 * don't bother re-reading it.					*/

		return;
	}

	entry->file_data->buffer = malloc(entry->file_data->__size);

	/* This code should seek to the location in the archive file							*
	 * contaning the file's data [P_FILE_DATA + ba_get_hdrlen + entry->file_data->__start]	*
	 * and read it into the buffer [entry->file_data->buffer]								*/

	fseek(arch->file, (P_FILE_DATA + __ba_get_hdrlen(arch->file) + entry->file_data->__start), SEEK_SET);		/* arch->file *should* be open */

	int byte;

	for (offset_t i = 0; i < entry->file_data->__size; i++)
	{
		byte = fgetc(arch->file);
		check(byte != EOF, "Unexpected end of archive file.");

		entry->file_data->buffer[i] = byte;
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
	__ba_load_dir_tree(&arch->__data_size, arch->header, &arch->entry_tree);

	return arch;

error:

	/*ba_close(arch);*/

	return NULL;
}

void __ba_load_dir_tree(fsize_t *total_size, char *orig_header, ba_Entry **first_entry)
{
	/* This function reads the XML header,			*
	 * and loads the files' metadata into a tree	*
	 * in memory. It then changes the first_entry	*
	 * to point to this tree.						*/

	check(total_size, "Null-pointer passed to __ba_load_dir_tree() for char fsize_t *total_size.");

	char *header = strdup(orig_header);		/* We need our own copy, because I think ezxml calls strtok on it */

	/* The root directory of the archive (ie. root node of the XML) */
	ezxml *root_dir = ezxml_parse_str(header, strlen(header));

	check(root_dir, "XML parser error: %s", ezxml_error(root_dir));

	__ba_process_xml_dir(total_size, root_dir, first_entry, NULL);		/* Run the recursive (self-calling) function that goes through all the directories and adds everything to the list */

	ezxml_free(root_dir);
	free(header);

	return;

error:
	return;
}

void __ba_process_xml_dir(fsize_t *total_size, ezxml *parent_xml, ba_Entry **first_entry, ba_Entry *parent_dir)
{
	/* Recursive function used by __ba_load_dir_tree()	*
	 * Adds all the files/dirs in the given directory	*
	 * to the directory tree which is in memory,		*
	 * which is pointed to by first_entry.				*
	 * 													*
	 * This function increments arch->__data_size.		*/

	ezxml *file_node = ezxml_child(parent_xml, "file");	/* A linked list of all the file nodes;			*/
	ezxml *dir_node  = ezxml_child(parent_xml, "dir");	/* A linked list of all the directory nodes.	*/

	ba_Entry   *entry;

	while (file_node)
	{
		entry = __ba_get_file_metadata(file_node, parent_dir);

		/* Add the file to the entry tree */
		bael_add( parent_dir ? &(parent_dir->child_entries) : first_entry, entry);		/* If we are in the top level directory and parent_dir is NULL, the pointer to parent_dir->child_entries would be false.		*/

		/* Increment the archive's total size */
		*total_size += entry->file_data->__size;

		file_node = file_node->next;
	}

	while (dir_node)
	{
		/* Get the directory's metadata */
		entry = __ba_get_dir_metadata(dir_node, parent_dir);

		/* Add the directory to the entry list */
		bael_add( parent_dir ? &(parent_dir->child_entries) : first_entry, entry);

		/* Run this function again, this time on the newly loaded directory */
		__ba_process_xml_dir(total_size, dir_node, &(entry->child_entries), entry);

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

	file_data->buffer      = NULL;						/* The file will only be loaded into memory if it is being modified, or if the source archive file is being overwritten. */
	file_data->__size      = atoi( ezxml_attr(file_node, "size")  );
	file_data->__start     = atoi( ezxml_attr(file_node, "start") );
	file_data->__old_start = -1;

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
	check(file_entry, "Null-pointer given to ba_extract() for BoxArchive *arch.");
	check(file_entry, "Null-pointer given to ba_extract() for ba_Entry *file_entry.");

	fsize_t file_size;

	/* Get the file's contents */
	uint8_t *data = ba_get_file_contents(arch, file_entry, &file_size);
	check(data, "Error getting file contents of \"%s\".", file_entry->path);

	/* Open the file to write to */
	FILE* out_file = fopen(dest, "w+");
	check(out_file, "File \"%s\" could not be opened for writing.", dest);

	for (fsize_t offset = 0; offset < file_size; offset++)
	{
		/* Write each byte out to the file */
		fputc(data[offset], out_file);
	}

	fclose(out_file);

	free(data);

	return 0;

error:
	return 1;
}

void ba_add(BoxArchive *arch, ba_Entry **parent_entry, ba_Entry *add_entry)
{
	/* This function addds the given entry				*
	 * into the given directory. Like its counterparts,	*
	 * changes arch->data_size.							*
	 *													*
	 * Note that this function expects that add_entry 	*
	 * is correctly filled in, otherwise there will be	*
	 * errors at later stages. You are urged to use		*
	 * ba_add_file() & ba_add_dir() unless you know		*
	 * what you're doing.								*/

	check(add_entry   , "Null pointer given for ba_Entry *add_entry to ba_add().");

	if (parent_entry)
	{
		/* If we're in a sub-direcotry */

		bael_add(&((*parent_entry)->child_entries), add_entry);
	}
	else
	{
		/* If we're in the root drectory */

		bael_add(&(arch->entry_tree), add_entry);
	}

	/* Increment the data size */
	arch->__data_size += add_entry->file_data->__size;

error:
	return;
}

ba_Entry* ba_add_file(BoxArchive *arch, ba_Entry **parent_entry, char *file_name, char *loc)
{
	/* Like ba_add() but fills in the struct for you.	*
	 * If 'loc' is NULL it creates a zero-byte file.	*/

	check(arch, "Null-pointer given to ba_add_file() for BoxArchive *arch.");

	ba_Entry *add_entry = malloc(sizeof(ba_Entry));

	check(add_entry, "Out of memory (malloc() returned NULL).");

	add_entry->type       = ba_EntryType_FILE;
	add_entry->__orig_loc = loc ? strdup(loc) : NULL;
	add_entry->path       = dupcat((parent_entry ? (*parent_entry)->path : ""), file_name, "", "");
	add_entry->name       = strdup(file_name);
	add_entry->file_data  = malloc(sizeof(ba_File));
	add_entry->parent_dir = (parent_entry ? *parent_entry : NULL);		/* Doesn't matter if it's NULL */
	add_entry->child_entries = NULL;

	check(add_entry->file_data, "Out of memory (malloc() returned NULL).");

	/* Fill in the file data */					/* These are ESSENTIAL. Without them __ba_create_archive_file() would crash and burn. */
	add_entry->file_data->buffer  = NULL;								/* Initialize the buffer with NULL, so that we know that the buffer is not loaded. */
	add_entry->file_data->__size  = loc ? ba_fsize(add_entry->__orig_loc) : 0;	/* We check that ba_fsize hasn't failed in a couple of lines time. I don't do it straight away to keep the code clear. */
	add_entry->file_data->__start = -1;					/* -1 for now, because we haven't been saved to a file yet. */
	add_entry->file_data->__old_start = -1;				/* See 'doc/howitworks.txt' for info about ->__old_start */

	check(add_entry->file_data->__size != -1, "ba_fsize() returned -1.");		/* Check that ba_fsize() hasn't failed. */

	arch->__data_size += add_entry->file_data->__size;				/* Increment the overall size by our size, so that other files can beadded to the NEW end of the data chunk */

	if (parent_entry)
	{
		/* If we're in a sub-direcotry */

		bael_add(&((*parent_entry)->child_entries), add_entry);
	}
	else
	{
		/* If we're in the root drectory */

		bael_add(&(arch->entry_tree), add_entry);
	}

	return add_entry;

error:
	return NULL;
}

ba_Entry* ba_add_dir(BoxArchive *arch, ba_Entry **parent_entry, char *dir_name)
{
	check(arch, "Null-pointer given to ba_add_file() for BoxArchive *arch.");

	/* Like ba_add_file() but for directories. */

	ba_Entry *add_entry = malloc(sizeof(ba_Entry));

	check(add_entry, "Out of memory (malloc() returned NULL).");

	add_entry->type       = ba_EntryType_DIR;
	add_entry->__orig_loc = NULL;
	add_entry->path       = dupcat((parent_entry ? (*parent_entry)->path : ""), dir_name, (dir_name[strlen(dir_name)-1] == BA_SEP[0] ? "" : BA_SEP), "");
	add_entry->name       = strdup(dir_name);
	add_entry->file_data  = NULL;
	add_entry->parent_dir = (parent_entry ? *parent_entry : NULL);		/* Doesn't matter if this is NULL */
	add_entry->child_entries = NULL;		/* For now... */

	if (parent_entry)
	{
		/* If we're in a sub-direcotry */

		bael_add(&((*parent_entry)->child_entries), add_entry);
	}
	else
	{
		/* If we're in the root drectory */

		bael_add(&(arch->entry_tree), add_entry);
	}

	return add_entry;

error:
	return NULL;
}

void ba_remove(BoxArchive *arch, ba_Entry **rm_entry)
{
	/* Removes (deletes) rm_entry from its archive. */

	check(arch, "Null-pointer given for 'BoxArchive *arch' to ba_remove().");
	check(*rm_entry != NULL, "Null-pointer given for 'ba_Entry **rm_entry' to ba_remove().");

	if ((*rm_entry)->type == ba_EntryType_FILE)
	{
		/* If the entry is a file */

		bael_remove  (arch, (*rm_entry));		/* We have to de-reference it since rm_entry is a double pointer. */
	}
	else if ((*rm_entry)->type == ba_EntryType_DIR)
	{
		bael_remove (arch, (*rm_entry));				/* Remove the directory from the entry tree; DO NOT DESTROY IT YET */
		bael_free ( &((*rm_entry)->child_entries) );	/* Free the tree of child entries */
	}
	if ((*rm_entry)->file_data)
	{
		arch->__data_size -= (*rm_entry)->file_data->__size;	/* Subtract the size of the file from the total */
		free((*rm_entry)->file_data);
	}

	ba_entry_free((*rm_entry));

	*rm_entry = NULL;

	return;

error:
	return;
}

void ba_move(BoxArchive *arch, ba_Entry *src_entry, ba_Entry **dest_entry)
{
	/* Moves the entry at *src_entry into *dest_dir.*
	 * Works for directories too.					*
	 * Currently 'BoxArchive *arch' is not used,	*
	 * but it may be needed in the future.			*/

	check(arch, "Null-pointer given for 'BoxArchive *arch' to ba_remove().");
	check(src_entry != NULL,  "Null-pointer given for 'ba_Entry *src_entry' to ba_move().");
	check(dest_entry != NULL, "Null-pointer given for 'ba_Entry *dest_entry' to ba_move().");

	bael_remove(arch, src_entry);		/* baEL_remove(). This doesn't do the fancy stuff that ba_remove does; this is what we want. */

	if ((*dest_entry)->type == ba_EntryType_DIR)
	{
		bael_add(&((*dest_entry)->child_entries), src_entry);
	}
	if ((*dest_entry)->type == ba_EntryType_FILE)
	{
		ba_add(arch, &((*dest_entry)->parent_dir), src_entry);
		ba_remove(arch, dest_entry);
		*dest_entry = src_entry;
	}

	return;

error:
	return;
}


fsize_t ba_treesize(ba_Entry *dir)
{
	if (! dir) return 0;

	fsize_t size = 0;

	__ba_treesize_rec_func(dir->child_entries, &size);

	return size;
}

void __ba_treesize_rec_func(ba_Entry *first_entry, fsize_t *size)
{
	ba_Entry *current = first_entry;

	while (current)
	{
		if (current->file_data)
		{
			*size += current->file_data->__size;
		}

		if (current->type == ba_EntryType_DIR)
		{
			__ba_treesize_rec_func(current->child_entries, size);
		}

		current = current->next;
	}
}

void __ba_create_header(BoxArchive *arch)
{
	/* This function recursiveley goes through an archive's	*
	 * entry tree, and creates an XML representation of it,	*
	 * which arch->header points to.						*/

	check(arch, "Null-pointer given to __ba_create_header().");

	ezxml *xml  = ezxml_new("header");
	check(xml, "Error creating XML header: %s", ezxml_error(xml));

	__ba_dir_entry_to_xml(xml, arch->entry_tree);

	if (arch->header)	free(arch->header);		/* Frees the old header if one exists */
	arch->header = 	ezxml_toxml(xml);

	ezxml_free(xml);

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

	return arch->entry_tree;	/* This is the real thing, not a copy, so be careful with it */

error:
	return NULL;
}

ba_Entry* ba_get(BoxArchive *arch, char *path)
{
	check(arch, "Null pointer given for BoxArchive *arch to ba_get().");
	check(path, "Null pointer given for char *orig_path to ba_get().");

	ba_Entry *return_entry = __ba_get_rec_func(arch->entry_tree, path);

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

uint8_t* ba_get_file_contents(BoxArchive *arch, ba_Entry *entry, fsize_t *size)
{
	/* This function returns a copy of the file's contents	*
	 * (irrelevant of their source), and sets *size to the	*
	 * size of the returned data. 							*
	 * For text files use ba_get_textfile_contents().		*/

	check(arch, "Null-pointer given to ba_get_file_contents() for 'BoxArchive *arch'.");
	check(entry, "Null-pointer given to ba_get_file_contents() for 'ba_Entry *entry'.");

	if (entry->type != ba_EntryType_FILE)
	{
		return NULL;
	}

	/* Set the size */
	*size = entry->file_data->__size;

	/* Load data into buffer if necessary */
	if (! entry->file_data->buffer)
	{
		if (entry->__orig_loc)
		{
			/* Load from source file if newly added */

			entry->file_data->buffer = malloc(entry->file_data->__size);

			FILE* sourcefile = fopen(entry->__orig_loc, "r");
			fread(entry->file_data->buffer, entry->file_data->__size, 1, sourcefile);		/* You are welcome to change this if you have a nicer way of loading the file into memory. */
			fclose(sourcefile);
		}
		else
		{
			/* Else load from archive */

			__ba_buffer_file(arch, entry);
		}
	}


	/* Return a copy of the buffer */
	uint8_t *copy = malloc(entry->file_data->__size);
	memcpy(copy, entry->file_data->buffer, entry->file_data->__size);

	return copy;

error:
	return NULL;
}

char* ba_get_textfile_contents(BoxArchive *arch, ba_Entry *entry)
{
	/* This function is a wrapper around ba_get_file_contents()	*
	 * which returns a null-terminated string instead of an		*
	 * array of bytes. Particularly useful for textfiles.		*/

	fsize_t size;
	char *data = (char*) ba_get_file_contents(arch, entry, &size);

	if (! data)
		return NULL;

	data = realloc(data, size+1);		/* Add a byte to the end, so that there's space for the null-term */

	data[size] = '\0';
	return data;
}

void ba_set_file_contents(BoxArchive *arch, ba_Entry *entry, uint8_t *data, fsize_t size)
{
	/* This function replaces a file entry's data 	*
	 * with a copy of uint8_t *data.				*/

	check(arch, "Null-pointer given to ba_get_file_contents() for 'BoxArchive *arch'.");
	check(entry, "Null-pointer given to ba_get_file_contents() for 'ba_Entry *entry'.");
	check(data, "Null-pointer given to ba_get_file_contents() for 'uint8_t *data'.");

	check(entry->type == ba_EntryType_FILE, "Entry is not a file!");

	/* Free the existing buffer if it exists.	*
	 * I'm not using realloc because I don't	*
	 * want to end up accidentally freeing		*
	 * the memory.								*/

	if (entry->file_data->buffer)
	{
		free(entry->file_data->buffer);
	}

	/* Store the size of the new data */
	entry->file_data->__size = size;

	/* Create enough memory */
	entry->file_data->buffer = malloc(size);
	memcpy(entry->file_data->buffer, data, size);

	return;

error:
	return;
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

	uint16_t length;
	fread(&length, sizeof(uint16_t), 1, file);

	return (hdrlen_t) ltos16(length);		/* no need to cast (hhdrlen_t = uint16_t) but still... */

error:
	return 0;
}

char* ba_get_header(BoxArchive *arch)
{
	/* Returns a copy of the 	*
	 * archive's XML header.	*/

	check(arch, "[ERROR] Null-pointer given to ba_get_header().");

	__ba_create_header(arch);		/* The current header may not necessarily be up to date; update it. */

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
		if (arch->entry_tree)	bael_free(&arch->entry_tree);

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
