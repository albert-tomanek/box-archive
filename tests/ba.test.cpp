#include <stdlib.h>

#define  NO_HIPPOMOCKS_NAMESPACE
#include <HippoMocks/hippomocks.h>
#include <UnitTest++/UnitTest++.h>

#include "../box_archive.h"
#include "../entrylist.h"

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
		ba_Entry **tmp;				// This is used by test_box_archive_new to store pointers to the initial entries so that test_box_archive_close can free them.
		BoxArchive *arch = test_box_archive_new(&tmp);
		
		ba_Entry *dummy_entry = test_file_entry_new("count.txt", "count.txt");
		ba_add(arch, &tmp[0], dummy_entry);						// Yes, we don't increment arch->__data_size manually as is required when using ba_add(), but it is not necessary for the purpouses of this test.
		
		CHECK(tmp[1]->next == dummy_entry);
		
		ba_entry_free(dummy_entry);
		test_box_archive_free(arch, &tmp);
	}
}
