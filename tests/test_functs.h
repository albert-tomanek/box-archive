#ifdef __cplusplus
extern "C" {
#endif

#ifndef __BOX_ARCHIVER_TEST_FUNCTS_H__
#define __BOX_ARCHIVER_TEST_FUNCTS_H__

#include "../box_archive.h"

BoxArchive* test_box_archive_new(ba_Entry ***init_entries);
void test_box_archive_free(BoxArchive *arch, ba_Entry ***init_entries);

ba_Entry *test_file_entry_new(char *path, char *name);	// Creates a test file entry with the file's contents in an in-memory buffer.

#endif

#ifdef __cplusplus
}
#endif
