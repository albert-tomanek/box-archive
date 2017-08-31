#include "tests.h"

void test_ba_add_file_subdir(void **state)
{
	/* Set up __wrap_ba_get_metadata */

	expect_string(__wrap_ba_get_metadata, dir_name, "./floppy.gif");

	will_return(__wrap_ba_get_metadata, 1034349302);	// atime
	will_return(__wrap_ba_get_metadata, 1034349303);	// mtime
	
	/* Set up ba_fsize */
	
	expect_string(__wrap_ba_fsize, path, "./floppy.gif");
	
	will_return(__wrap_ba_fsize, 48);
	
	//

	/* Test the function */

	BoxArchive *arch = *state;
	ba_Entry *parent_dir = arch->entry_tree;

	ba_Entry *new = ba_add_file(arch, &parent_dir, "file.dat", "./floppy.gif");		// &arch->entry_tree = the 'myDir' directory
	
	assert_ptr_not_equal(new, NULL);
	
	assert_true          (new->type == ba_EntryType_FILE);
	assert_string_equal  (new->path, "myDir" BA_SEP "file.dat");
	assert_string_equal  (new->name, "file.dat");
	
	assert_ptr_not_equal (new->meta, NULL);
	assert_int_equal     (new->meta->atime, 1034349302);
	assert_int_equal     (new->meta->mtime, 1034349303);
	
	assert_ptr_not_equal (new->file_data, NULL);
	assert_true          (new->file_data->__size == 48);
	
	assert_ptr_equal     (new->parent_dir, parent_dir);
	assert_ptr_equal     (new->child_entries, NULL);
}
