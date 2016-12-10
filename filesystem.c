#include <stdlib.h>
#include <string.h>

#include "dbg.h"
#include "dupcat.h"
#include "box_archive.h"
#include "filesystem.h"

#ifdef __unix__
	#include <unistd.h>
	#include <sys/stat.h>	/* For mkdir */

	void ba_mkdir(char *parent_dir, ba_Entry *dir)
	{
		char *full_path = NULL;

		check(dir, "Null-pointer given for ba_Entry *dir to __ba_load_header().");
		check(dir->type == ba_EntryType_DIR, "Filesystem entry given to ba_mkdir is not a directory!");

		full_path = dupcat(parent_dir, (parent_dir[strlen(parent_dir)-1] == BA_SEP[0] ? "" : BA_SEP), dir->path);		/* eg. "/tmp" + "/" + "myProg" */

		mkdir(full_path, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) );				/* Note to self: tar extracts dirs as drwxr-xr-x */

		free(full_path);

		return;

	error:
		if (full_path)	free(full_path);

		return;
	}

#endif


#if defined(_WIN32) || defined(WIN32)

/* Windows support not implemented yet */

#endif
