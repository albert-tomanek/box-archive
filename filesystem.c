/* Copyright (C) 2017  Albert Tomanek *
 * For license see LICENSE.txt        */

#include <stdlib.h>
#include <string.h>

#include "dbg.h"
#include "types.h"
#include "dupcat.h"
#include "box_archive.h"
#include "entrylist.h"
#include "metadata.h"
#include "filesystem.h"

#ifdef __unix__
	/* Unix system libraries which I'm not too confident using... 	*/

	#include <stddef.h>
	#include <unistd.h>
	#include <dirent.h>
	#include <sys/types.h>
	#include <sys/stat.h>	/* For mkdir and fstat */

	/* 'Private' declarations */

	size_t dirent_entry_size(char *path);
	void __rec_getdir_func(char *path, char *root_dir_path, ba_Entry **first_entry, ba_Entry *parent_entry, fsize_t *data_size);

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

	void ba_load_fs_tree(char *orig_root_path, ba_Entry **first_entry, fsize_t *data_size)
	{
		/* This function loads the filesystem hierarchy		*
		 * at *orig_root_path into an entry tree, 			*
		 * and points *first_entry to it.					*
		 * if *data_size is not NULL, it will add the size 	*
		 * of each file it loads to it.						*
		 * 													*
		 * Loaded files are stored as entry->__orig_loc;	*
		 * entry->file_data->buffer is NULL.				*/

		char *root_path = NULL;

		check(first_entry, "Null-pointer given to ba_load_fs_tree() for 'ba_Entry **first_entry'");

		root_path = dupcat(orig_root_path, (orig_root_path[strlen(orig_root_path)-1] == BA_SEP[0] ? "" : BA_SEP), "", ""); 	/* just ads a '/' to the end of the root path if it isn't there. Eg. makes '/tmp/test/' from ''/tmp/test'. */

		__rec_getdir_func("", root_path, first_entry, NULL, data_size);

		free(root_path);
		return;

	error:
		if (root_path)	free(root_path);
		return;
	}

	void __rec_getdir_func(char *path, char *root_path, ba_Entry **first_entry, ba_Entry *parent_entry, fsize_t *data_size)		/* 'root_dir_path' is the path of the root of the archive, while 'path' is the path of the current entry in the archive. */
	{
		/* Recursive function used	*
		 * by ba_load_fs_tree().	*/

		char *full_path = dupcat(root_path, path, "", "");				/* This is the path of the archive's root directory, + the path of the directory to process.	*/
		DIR  			 *dirent_dir 	= opendir(full_path);
		check(dirent_dir != NULL, "Error getting contents of '%s'.", path);

		struct dirent 	*dirent_entry 	= malloc( dirent_entry_size(path) );
		struct dirent   *dirent_result;	/* Will point to dirent_entry unless we encounter the end of the directory, where it'll be NULL. */

		struct ba_Entry *current;

		while(	readdir_r(dirent_dir, dirent_entry, &dirent_result) == 0		/* NOTE: As of 2016-12-30 (or Lubuntu 16.10), readdir_r seems to be deprecated (see 'man readdir'),	*/
				&& (dirent_result != NULL) )									/*       but there doesn't appear to be an up-to-date replacement.									*/
		{																		/*       Someone please update it/contact me if you find the correct and up-to-date way to do this.	*/
			if (! ( strcmp(dirent_entry->d_name, ".")
				&&  strcmp(dirent_entry->d_name, "..")))
			{
				/* We don't want the . and .. directories in our archive.	*/
				continue;
			}

			/* Create an entry */
			current = calloc(1, sizeof(ba_Entry));		/* Allocate memory to hold the new entry */
			check(current, "Out of memory.");

			/* Entry-type dependent */
			if (dirent_entry->d_type == DT_REG)		/* REGular file */
			{
				current->type = ba_EntryType_FILE;
				current->name = strdup(dirent_entry->d_name);
				current->path = dupcat(path, current->name, "", "");
				current->file_data     = malloc(sizeof(ba_File));
				current->parent_dir    = parent_entry;
				current->child_entries = NULL;
				current->__orig_loc    = dupcat(root_path, current->path, "", "");		/* eg. '/tmp/test/' + 'directory/file.dat'. This string us used so that ba_save() knows where to read the source file from when writing to the archive. */

				check(current->file_data, "Out of memory (malloc() returned NULL).");

				/* File metadata */

				current->meta = ba_get_meta(current->__orig_loc);

				/* File data */

				current->file_data->buffer  = NULL;
				current->file_data->__size  = ba_fsize(current->__orig_loc);		/* These are ESSENTIAL. Without them __ba_create_archive_file() would crash and burn. */
				current->file_data->__start = *data_size;
				current->file_data->__old_start = -1;

				check(current->file_data->__size != -1, "ba_fsize() returned -1.");		/* Check that ba_fsize() hasn't failed. */

				if (data_size)	*data_size += current->file_data->__size;			/* If they dive us a null-pointer to data_size, we'd fet a seg-fault if we tried to increment it. */
			}
			if (dirent_entry->d_type == DT_DIR)		/* Directory */
			{
				current->type = ba_EntryType_DIR;
				current->name = strdup(dirent_entry->d_name);
				current->path = dupcat(path, current->name, BA_SEP, "");	/* The path of the directory WITHIN THE ARCHIE */
				current->file_data     = NULL;							/* The file hasn't been saved in an archive yet */
				current->parent_dir    = parent_entry;
				current->child_entries = NULL;
				current->__orig_loc    = dupcat(root_path, current->path, "", "");		/* eg. '/tmp/test/' + 'directory/file.dat'. This string us used so that ba_save() knows where to read the source file from when writing to the archive. */

				/* Directory metadata */
				current->meta = ba_get_meta(current->__orig_loc);

				/* Run on the subdirectory */
				__rec_getdir_func(current->path, root_path, &(current->child_entries), current, data_size);
			}


			bael_add(first_entry, current);		/* Add the newly created entry to the entry tree */

			/* Note I'm not doing free(current) here,		*
			 * because it is now part of our entry tree!	*/
		}

		free	(full_path);
		free	(dirent_entry);
		closedir(dirent_dir);

		return;

	error:
		if (full_path)		free	(full_path);
		if (dirent_dir)		closedir(dirent_dir);	/* When I did free(dirent_entry), it gave me an error -- so I removed it.	*/

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

	ba_Meta* ba_get_meta(char *path)
	{
		/* Returns a struct containing the metadata for the given file	*/

		ba_Meta *meta = malloc(sizeof(ba_Meta));
		struct stat entry_stat;

		check(meta, "Out of memory.");

		/* Ensure that the entry exists */
		if (stat(path, &entry_stat) != 0)
		{
			log_err("Filesystem entry \"%s\" not found.", path);
			goto error;
		}

		meta->atime = entry_stat.st_atime;
		meta->mtime = entry_stat.st_mtime;

		return meta;

	error:
		return NULL;
	}

	fsize_t ba_fsize(char *loc)
	{
		/* From: http://www.securecoding.cert.org/confluence/plugins/servlet/mobile#content/view/42729539 */

		struct stat file_stat;

		/* Ensure that it exists */
		if (stat(loc, &file_stat) != 0)
		{
			log_err("Filesystem entry \"%s\" not found.", loc);
			goto error;
		}

		/* Ensure that is is a regular file */
		if ( (!S_ISREG(file_stat.st_mode)) )
		{
			log_err("Filesystem entry \"%s\" is not a file.", loc);
			goto error;
		}

		return (fsize_t) file_stat.st_size;

	error:
		return -1;
	}

#endif


#if defined(_WIN32) || defined(WIN32)

	/* Windows support not implemented yet */

	/* I fear the Windows NT libraries will be even messier	*
	 * to use than the Unix ones...	:-(						*/

#endif
