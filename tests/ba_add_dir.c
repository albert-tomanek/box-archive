#include "tests.h"

void test_ba_add_dir(void **state)
{
	/* Set up __wrap_ba_get_metadata */

	expect_string(__wrap_ba_get_metadata, dir_name, "newDir");

	will_return(__wrap_ba_get_metadata, 1034349301);	// atime
	will_return(__wrap_ba_get_metadata, 1034349300);	// mtime

	/* Test the function */

	BoxArchive *arch = *state;
	ba_Entry *parent_dir = arch->entry_tree;

	ba_Entry *new = ba_add_dir(arch, &parent_dir, "newDir");		// &arch->entry_tree = the 'myDir' directory

	assert_ptr_not_equal(new, NULL);
	
	assert_true          (new->type == ba_EntryType_DIR);
	assert_ptr_equal     (new->__orig_loc, NULL);
	assert_string_equal  (new->path, "myDir" BA_SEP "newDir");
	assert_string_equal  (new->name, "newDir");
	
	assert_ptr_not_equal (new->meta, NULL);
	assert_int_equal     (new->meta->atime, 1034349301);
	assert_int_equal     (new->meta->mtime, 1034349300);
	
	assert_ptr_equal     (new->file_data, NULL);
	assert_ptr_equal     (new->parent_dir, parent_dir);
	assert_ptr_equal     (new->child_entries, NULL);
}
