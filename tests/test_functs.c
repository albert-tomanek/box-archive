#include <stdlib.h>
#include <string.h>

#include "../box_archive.h"
#include "test_functs.h"

/* Test box archive structure:	*
 * 								*
 *    +-------+  +-----------+	*
 *  ->| myDir |->| hello.txt |	*
 *    +-------+  +-----------+	*
 *        |    					*
 *  +------------+				*
 *  | floppy.gif |				*
 *  +------------+				*
 * 								*/

#define TEST_ENTRIES 3

BoxArchive* test_box_archive_new(ba_Entry ***init_entries)
{
	/* Allocate the memory */

	ba_Entry *ent1      = calloc(1, sizeof(ba_Entry));
	ba_File  *ent1_file = calloc(1, sizeof(ba_File));
	ba_Meta  *ent1_meta = calloc(1, sizeof(ba_Meta));

	ba_Entry *ent2      = calloc(1, sizeof(ba_Entry));
	ba_File  *ent2_file = calloc(1, sizeof(ba_File));
	ba_Meta  *ent2_meta = calloc(1, sizeof(ba_Meta));

	ba_Entry *ent3      = calloc(1, sizeof(ba_Entry));
	ba_File  *ent3_file = calloc(1, sizeof(ba_File));
	ba_Meta  *ent3_meta = calloc(1, sizeof(ba_Meta));

	/* Entry 1 */

	ent1->type = ba_EntryType_DIR;
	ent1->__orig_loc = NULL;
	ent1->path = "myDir";
	ent1->name = "myDir";
	ent1->meta = ent1_meta;
	ent1->file_data  = NULL;	/* No file data because it's a directory */
	ent1->parent_dir = NULL;
	ent1->child_entries = ent2;
	ent1->next = ent3;

	ent1->meta->atime = (time_t) 123456;
	ent1->meta->atime = (time_t) 123321;

	/* Entry 2 */

	ent2->type = ba_EntryType_FILE;
	ent2->__orig_loc = "./floppy.gif";
	ent2->path = "myDir/floppy.gif";
	ent2->name = "floppy.gif";
	ent2->meta = ent2_meta;
	ent2->file_data  = ent2_file;
	ent2->parent_dir = ent1;
	ent2->child_entries = NULL;
	ent2->next = NULL;

	ent2_file->buffer  = NULL;
	ent2_file->__size  = 629;	/* ls -l floppy.gif */
	ent2_file->__start = -1;
	ent2_file->__old_start = -1;

	ent2->meta->atime = (time_t) 123456789;
	ent2->meta->atime = (time_t) 12345678;

	/* Entry 3 */

	ent3->type = ba_EntryType_FILE;
	ent3->__orig_loc = NULL;
	ent3->path = "hello.txt";
	ent3->name = "hello.txt";
	ent3->meta = ent3_meta;
	ent3->file_data  = ent3_file;
	ent3->parent_dir = NULL;
	ent3->child_entries = NULL;
	ent3->next = NULL;

	ent3_file->buffer  = (uint8_t *) strdup("Hello, world!\nGoodbye, world!");
	ent3_file->__size  = strlen((char *) ent3_file->buffer) + 1;		/* +1 because strlen does not count the null-term */
	ent3_file->__start = -1;
	ent3_file->__old_start = -1;

	ent3->meta->atime = (time_t) 5555555555;
	ent3->meta->atime = (time_t) 5555555555;

	/* Archive */

	BoxArchive *arch = malloc(sizeof(BoxArchive));

	arch->loc        = NULL;
	arch->header     = NULL;
	arch->file       = NULL;
	arch->entry_tree = ent1;
	arch->__data_size= 659;
	
	/* Make a null-terminated array containing pointers to the entries	*
	 * we just created, so that we can free them later.					*/
	ba_Entry **__init_entries = calloc(TEST_ENTRIES + 1, sizeof(ba_Entry *));
	__init_entries[0] = ent1;
	__init_entries[1] = ent2;
	__init_entries[2] = ent3;
	*init_entries = __init_entries;

	return arch;
}

void test_box_archive_free(BoxArchive *arch, ba_Entry ***init_entries)
{
	/* Clean up the entries that were in the archive initially */
	for (ba_Entry **ent = *init_entries; *ent != NULL; ent++)
	{
		if ((*ent)->file_data) free((*ent)->file_data->buffer);
		free((*ent)->file_data);		// Doesn't matter if we free NULL
		free((*ent)->meta);
		free(*ent);
	}
	
	free(*init_entries);
	*init_entries = NULL;
	
	/* Clean up the archive itsself */
	if (arch->file) fclose(arch->file);
	free(arch->header);
	free(arch->loc);

	free(arch);
}

ba_Entry *test_file_entry_new(char *path, char *name)
{
	ba_Entry *entry = calloc(1, sizeof(ba_Entry));
	
	entry->type = ba_EntryType_FILE;
	
	entry->__orig_loc = NULL;
	
	entry->path = strdup(path);
	entry->path = strdup(name);
	
	entry->meta = malloc(sizeof(ba_Entry));
	entry->meta->atime = 625093865;
	entry->meta->mtime = 437436845;
	
	entry->file_data = calloc(1, sizeof(ba_File));
	entry->file_data->buffer = strdup("1234567890");
	entry->file_data->__size = 11;
	entry->file_data->__start     = -1;
	entry->file_data->__old_start = -1;
	
	entry->parent_dir    = NULL;
	entry->child_entries = NULL;
	
	entry->next = NULL;
	
	return entry;
}
