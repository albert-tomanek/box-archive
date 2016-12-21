#include <stdlib.h>
#include <string.h>

#include "dbg.h"
#include "dupcat.h"
#include "box_archive.h"
#include "entrylist.h"
#include "filesystem.h"

#ifdef __unix__
	/* Unix system libraries which I'm not too confident using... 	*/

	#include <stddef.h>
	#include <unistd.h>
	#include <dirent.h>
	#include <sys/stat.h>	/* For mkdir */
	#include <sys/types.h>

	/* 'Private' declarations */

	size_t dirent_entry_size(char *path);
	void __rec_getdir_func(char *path, ba_Entry **first_entry, ba_Entry *parent_entry);

	/* Functions */

	void ba_mkdir(char *parent_dir, ba_Entry *dir)
	{
		char *full_path = NULL;

		check(dir, "Null-pointer given for ba_Entry *dir to __ba_load_header().");
		check(dir->type == ba_EntryType_DIR, "Filesystem entry given to ba_mkdir is not a directory!");

		full_path = dupcat(parent_dir, (parent_dir[strlen(parent_dir)-1] == BA_SEP[0] ? "" : BA_SEP), dir->path, "");		/* eg. "/tmp" + "/" + "myProg" */

		mkdir(full_path, (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) );				/* Note to self: tar extracts dirs as drwxr-xr-x */

		free(full_path);

		return;

	error:
		if (full_path)	free(full_path);

		return;
	}

	void ba_load_fs_tree(char *path, ba_Entry **first_entry)
	{
		/* check(**first_entry, ...)	gave an error for some reason, else I would have used it. */

		__rec_getdir_func(path, first_entry, NULL);

		return;

	error:
		return;
	}

	void __rec_getdir_func(char *path, ba_Entry **first_entry, ba_Entry *parent_entry)
	{
		DIR  			*dirent_dir 	= opendir(path);
		check(dirent_dir, "Error getting contents of '%s'.", path);

		struct dirent 	*dirent_entry 	= malloc( dirent_entry_size(path) );
		struct dirent   *dirent_result;	/* Will point to dirent_entry unless we encounter the end of the directory, where it'll be NULL. */

		struct ba_Entry *current;

		while(	readdir_r(dirent_dir, dirent_entry, &dirent_result) == 0
				&& (dirent_result != NULL) )
		{
			if (! ( strcmp(dirent_entry->d_name, ".")
				&&  strcmp(dirent_entry->d_name, "..")))
			{
				/* We don't want the . and .. directories in our archive.	*/
				continue;
			}

			current = calloc(1, sizeof(ba_Entry));
			check(current, "Out of memory.");

			if (dirent_entry->d_type == DT_REG)		/* REGular fule */
			{
				current->type = ba_EntryType_FILE;
				current->name = strdup(dirent_entry->d_name);
				current->path = dupcat(path, (path[strlen(path)-1] == BA_SEP[0] ? "" : BA_SEP), current->name, "");
				current->file_data     = NULL;							/* The file hasn't been added to an archive yet */
				current->parent_dir    = parent_entry;
				current->child_entries = NULL;
			}
			if (dirent_entry->d_type == DT_DIR			/* Directory */ )
			{
				current->type = ba_EntryType_DIR;
				current->name = strdup(dirent_entry->d_name);
				current->path = dupcat(path, (path[strlen(path)-1] == BA_SEP[0] ? "" : BA_SEP), current->name, BA_SEP);
				current->file_data     = NULL;							/* The file hasn't been saved in an archive yet */
				current->parent_dir    = parent_entry;
				current->child_entries = NULL;

				/* Run on the subdirectory */
				__rec_getdir_func(current->path, &(current->child_entries), current);
			}

			bael_add(first_entry, current);

			/* Note I'm not doing free(current) here,		*
			 * Because it is now part of our entry tree!	*/
		}

		free	(dirent_entry);
		closedir(dirent_dir);

		return;

	error:
		return;
	}

	size_t dirent_entry_size(char *path)
	{
		/* NOT MY CODE -						*
		 * man readdir(3) reccommended I should	*
		 * use this.							*/

		int name_max = pathconf(path, _PC_NAME_MAX);
		if (name_max == -1)         /* Limit not defined, or error */
			name_max = 255;         /* Take a guess */
		return offsetof(struct dirent, d_name) + name_max + 1;
	}

#endif


#if defined(_WIN32) || defined(WIN32)

	/* Windows support not implemented yet */

	/* I fear the Windows NT libraries will be even messier	*
	 * to use than the Unix ones...	:-(						*/

#endif
