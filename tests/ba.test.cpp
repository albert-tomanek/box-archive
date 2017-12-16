#include <stdlib.h>
#include <time.h>

#define  NO_HIPPOMOCKS_NAMESPACE
#include <HippoMocks/hippomocks.h>
#include <UnitTest++/UnitTest++.h>

#include "../box_archive.h"
#include "../entrylist.h"
#include "../filesystem.h"

#include "test_functs.h"

SUITE (BoxArchive)
{
	TEST (ba_new)
	{
		BoxArchive *arch = ba_new();
		REQUIRE CHECK(arch != NULL);		// This would only fail if malloc fails, in which case the function would already have complained into stderr

		/* Check that there is no junk */
		CHECK(arch->loc          == NULL);
		CHECK(arch->header       == NULL);
		CHECK(arch->file         == NULL);
		CHECK(arch->entry_tree   == NULL);
		CHECK_EQUAL(arch->__data_size, 0);
		
		ba_close(arch);
	}
	
	TEST (ba_close)
	{
		MockRepository mocks;
		mocks.autoExpect = false;	// We don't care about the order the mocks are called in.
		
		BoxArchive *arch = ba_new();
		arch->loc = strdup("loc");
		arch->header = strdup("header");
		arch->file = (FILE *) 0xf01dab1e;
		arch->entry_tree = (ba_Entry *) 0xba5eba11;
		
		mocks.ExpectCallFunc(free).With(arch->loc);
		mocks.ExpectCallFunc(free).With(arch->header);
		mocks.ExpectCallFunc(fclose).With(arch->file).Return(0);	// Return sth so that the actual func doesn't have to be called
		mocks.ExpectCallFunc(bael_free).With(&arch->entry_tree);	// Don't do anyth but don't call the func either.
		mocks.ExpectCallFunc(free).With(arch);
		
		ba_close(arch);
	}
	
	TEST (ba_get)
	{
		ba_Entry **tmp;				// This is used by test_box_archive_new to store pointers to the initial entries so that test_box_archive_close can free them.
		BoxArchive *arch = test_box_archive_new(&tmp);
		
		CHECK(ba_get(arch, "myDir") == tmp[0]);		// BOTH of these have to work -- we don't know whether they'll pass us the slash at the end of the stirng.
		//CHECK(ba_get(arch, "myDir/") == tmp[0]);	// THIS ISN'T ALLOWED
		CHECK(ba_get(arch, "myDir/floppy.gif") == tmp[1]);
		CHECK(ba_get(arch, "") == NULL);
		CHECK(ba_get(arch, "UIAF:HuifU;hg:FEIUuojhFUEad") == NULL);
		
		test_box_archive_free(arch, &tmp);
	}

	TEST (ba_add)
	{
		ba_Entry **tmp;				// This is used by test_box_archive_new to store pointers to the initial entries so that test_box_archive_close can free them.
		BoxArchive *arch = test_box_archive_new(&tmp);
		
		ba_Entry *dummy_entry = test_file_entry_new("count.txt", "count.txt");
		ba_add(arch, NULL, dummy_entry);								// Yes, we don't increment arch->__data_size manually as is required when using ba_add(), but it is not necessary for the purpouses of this test.
		
		CHECK(tmp[2]->next == dummy_entry);
		
		ba_entry_free(dummy_entry);
		test_box_archive_free(arch, &tmp);
	}

	TEST (ba_add_IntoSubDirectory)
	{
		ba_Entry **tmp;
		BoxArchive *arch = test_box_archive_new(&tmp);
		
		ba_Entry *dummy_entry = test_file_entry_new("count.txt", "count.txt");
		ba_add(arch, &tmp[0], dummy_entry);						// Yes, we don't increment arch->__data_size manually as is required when using ba_add(), but it is not necessary for the purpouses of this test.
		
		CHECK(tmp[1]->next == dummy_entry);
		
		ba_entry_free(dummy_entry);
		test_box_archive_free(arch, &tmp);
	}

	TEST (ba_add_file)
	{
		MockRepository mocks;
		ba_Entry **tmp;
		BoxArchive *arch = test_box_archive_new(&tmp);
		char *file_path = "./floppy.gif";
		ba_Meta *meta = (ba_Meta *) malloc(sizeof(ba_Meta));
		size_t fsize = 0x73697a65;
		size_t old_arch_size = arch->__data_size;

		mocks.ExpectCallFunc(ba_get_metadata).Return(meta);
		mocks.ExpectCallFunc(ba_fsize).Return(fsize);
		mocks.ExpectCallFunc(bael_add);
		
		ba_Entry *new_entry = ba_add_file(arch, &tmp[0], "file.dat", file_path);
		
		CHECK(new_entry != NULL);
		CHECK(new_entry->type == ba_EntryType_FILE);
		CHECK(! strcmp(new_entry->__orig_loc, "./floppy.gif"));
		CHECK(new_entry->__orig_loc != file_path);		// Check that the string was actually duplicated and not just the pointer copied
		CHECK_EQUAL("myDir" BA_SEP "file.dat", new_entry->path);
		CHECK_EQUAL("file.dat", new_entry->name);
		CHECK(new_entry->meta == meta);
		CHECK(new_entry->file_data != NULL);
		CHECK(new_entry->file_data->buffer  == NULL);
		CHECK(new_entry->file_data->__size  == fsize);
		CHECK(new_entry->file_data->__start == -1);
		CHECK(new_entry->file_data->__old_start == -1);
		CHECK(new_entry->child_entries == NULL);
		
		CHECK(arch->__data_size == old_arch_size + fsize);		// Check that they've remembered to add the file's size to the archive's total size.
		
		ba_entry_free(new_entry);
		test_box_archive_free(arch, &tmp);
	}

	TEST (ba_add_file_NewZeroByteFile)
	{
		MockRepository mocks;
		ba_Entry **tmp;
		BoxArchive *arch = test_box_archive_new(&tmp);
		ba_Meta *meta = (ba_Meta *) malloc(sizeof(ba_Meta));
		char *file_path = "./floppy.gif";
		size_t fsize = 0x73697a65;
		size_t old_arch_size = arch->__data_size;

		mocks.ExpectCallFunc(ba_meta_default).Return(meta);
		mocks.ExpectCallFunc(bael_add);
		
		ba_Entry *new_entry = ba_add_file(arch, &tmp[0], "file.dat", NULL);
		
		CHECK(new_entry != NULL);
		CHECK(new_entry->type == ba_EntryType_FILE);
		CHECK(new_entry->__orig_loc == NULL);			// No orig_loc since you don't need to read any data for a zero byte file
		CHECK_EQUAL("myDir" BA_SEP "file.dat", new_entry->path);
		CHECK_EQUAL("file.dat", new_entry->name);
		CHECK(new_entry->meta == meta);
		CHECK(new_entry->file_data != NULL);
		CHECK(new_entry->file_data->buffer  == NULL);
		CHECK(new_entry->file_data->__size  ==  0);
		CHECK(new_entry->file_data->__start == -1);
		CHECK(new_entry->file_data->__old_start == -1);
		CHECK(new_entry->child_entries == NULL);
		
		CHECK(arch->__data_size == old_arch_size);		// Check that the archive's total size hasn't changed; we are adding a ZERO BYTE file after all.
		
		ba_entry_free(new_entry);
		test_box_archive_free(arch, &tmp);
	}

	TEST (ba_add_dir)
	{
		MockRepository mocks;
		ba_Entry **tmp;
		BoxArchive *arch = test_box_archive_new(&tmp);
		ba_Meta *meta = (ba_Meta *) malloc(sizeof(ba_Meta));

		mocks.ExpectCallFunc(ba_meta_default).Return(meta);
		mocks.ExpectCallFunc(bael_add);
		
		ba_Entry *new_entry = ba_add_dir(arch, &tmp[0], "new.dir");
		
		CHECK(new_entry != NULL);
		CHECK(new_entry->type == ba_EntryType_DIR);
		CHECK(new_entry->__orig_loc == NULL);
		CHECK_EQUAL("myDir" BA_SEP "new.dir", new_entry->path);
		CHECK_EQUAL("new.dir", new_entry->name);
		CHECK(new_entry->meta == meta);
		CHECK(new_entry->file_data == NULL);		// Not a file
		CHECK(new_entry->child_entries == NULL);	// The dir has only just been created, so there should be no entries in it yet.
		
		ba_entry_free(new_entry);
		test_box_archive_free(arch, &tmp);
	}
	
	uint8_t* __MOCK_ba_get_file_contents(BoxArchive *arch, ba_Entry *entry, fsize_t *size)
	{
		*size = entry->file_data->__size;
		return NULL;
	}
	
	TEST (ba_extract)
	{
		MockRepository mocks;
		ba_Entry **tmp;
		BoxArchive *arch = test_box_archive_new(&tmp);
		ba_Entry *entry = tmp[1];
		uint8_t *data = (uint8_t *) malloc(entry->file_data->__size);
		FILE *file = (FILE *) 0x46494c45;
		char *dest = "./out.dat";

		mocks.ExpectCallFunc(ba_get_file_contents).Do(__MOCK_ba_get_file_contents).Return(data);
		mocks.ExpectCallFunc(fopen).Return(file);
		mocks.ExpectCallFunc(fwrite).With(data, 1, entry->file_data->__size, file).Return(entry->file_data->__size);	// .Return(...) : fwrite returns the number of items (in this case bytes) written.
		mocks.ExpectCallFunc(fclose).With(file).Return(0);
		mocks.ExpectCallFunc(ba_write_metadata).With(dest, entry->meta);
		
		int rc = ba_extract(arch, entry, dest);

		CHECK(rc == 0);
		
		test_box_archive_free(arch, &tmp);
	}
}
