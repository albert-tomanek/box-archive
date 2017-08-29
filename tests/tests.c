/* NOTE: I only check for significant things.				*
 *       I don't check for null-pointer handling because:	*
 *		 A) There are too many pointers to check.			*
 *		 B) The pointer should be checked at runtime using	*
 *		    the check() macro.								*
 *		 C) I have much more fun things to be doing with my	*
 *		    time.											*/

#include <string.h>
#include <stdarg.h>		// These are all here for cmocka to function
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "../box_archive.h"
#include "tests.h"

/* Setup and teardown functions */

BoxArchive* test_box_archive_new()
{
	//   [myDir]---[hello.txt]
	//      |
	// [floppy.gif]
	
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

	return arch;

}

void test_box_archive_free(BoxArchive *arch)
{
	/* Entry 3 */
	free(arch->entry_tree->next->file_data->buffer);
	free(arch->entry_tree->next->file_data);
	free(arch->entry_tree->next->meta);
	free(arch->entry_tree->next);

	/* Entry 2 */
	free(arch->entry_tree->child_entries->file_data->buffer);
	free(arch->entry_tree->child_entries->file_data);
	free(arch->entry_tree->child_entries->meta);
	free(arch->entry_tree->child_entries);

	/* Entry 1 */		// Entry 1 is a directory and so we do not need to free the file_data structure
	free(arch->entry_tree->meta);
	free(arch->entry_tree);

	if (arch->file) fclose(arch->file);
	free(arch->header);
	free(arch->loc);

	free(arch);
}

/* Wrap functions */

ba_Meta *__wrap_ba_get_metadata(char *dir_name)
{
	check_expected(dir_name);

	ba_Meta *meta = malloc(sizeof(ba_Meta));

	meta->atime   = mock();
	meta->mtime   = mock();

	return meta;
}

fsize_t __wrap_ba_fsize(char *loc)
{
	check_expected(loc);
	
	return mock();
}

//void test_myFunc(void **state)			// unused parameter
//{
//	/* State the parameters that __wrap_fgets expexts to get */
//	expect_any(__wrap_fgets, s);								// We don't check for the values of 's' and 'size' bacause these will vary.
//	expect_any(__wrap_fgets, size);
//	expect_memory(__wrap_fgets, stream, stdin, sizeof(FILE));	// __wrap_fgets expects the 'stream' parameter to be stdin.
//
//	will_return(__wrap_fgets, "42");	// The first time mock() is called in __wrap_fgets, it will return "42"
//
//	int return_val = myFunc(8);
//
//	assert_int_equal(return_val, 50);
//
//}

int main()
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_ba_new),
		cmocka_unit_test(test_ba_add_dir),
		cmocka_unit_test(test_ba_add_file)
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
