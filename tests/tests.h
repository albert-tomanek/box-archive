#ifndef __BOX_ARCHIVER_TESTS_H__
#define __BOX_ARCHIVER_TESTS_H__

#include <string.h>
#include <stdarg.h>		// These are all here for cmocka to function
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "../box_archive.h"

/* From tests.c */

BoxArchive* test_box_archive_new();
void test_box_archive_free(BoxArchive *arch);

/* Tested functions */
void test_ba_new(void **state);
void test_ba_add_dir(void **state);
void test_ba_add_file_subdir(void **state);

#endif
