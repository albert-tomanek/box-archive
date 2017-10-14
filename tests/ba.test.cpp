#define  NO_HIPPOMOCKS_NAMESPACE
#include <HippoMocks/hippomocks.h>
#include <UnitTest++/UnitTest++.h>

#include <stdlib.h>

#include "../box_archive.h"
#include "../entrylist.h"

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
		arch->entry_tree = (ba_Entry *) 0xba55ba11;
		
		mocks.ExpectCallFunc(free).With(arch->loc);
		mocks.ExpectCallFunc(free).With(arch->header);
		mocks.ExpectCallFunc(fclose).With(arch->file).Return(0);	// Return sth so that the actual func doesn't have to be called
		mocks.ExpectCallFunc(bael_free).With(&arch->entry_tree);	// Don't do anyth but don't call the func either.
		mocks.ExpectCallFunc(free).With(arch);
		
		ba_close(arch);
	}
}
