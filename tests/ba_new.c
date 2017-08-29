#include "tests.h"

void test_ba_new(void **state)
{
	BoxArchive *arch = ba_new();
	assert_ptr_not_equal(arch, NULL);		// This would only fail if malloc fails, in which case the function would already have complained into stderr

	/* Check that there is no junk */
	assert_ptr_equal(arch->loc,         NULL);
	assert_ptr_equal(arch->header,      NULL);
	assert_ptr_equal(arch->file,        NULL);
	assert_ptr_equal(arch->entry_tree,  NULL);
	assert_int_equal(arch->__data_size, 0);
}
