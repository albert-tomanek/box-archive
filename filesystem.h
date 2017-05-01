/* Copyright (C) 2017  Albert Tomanek *
 * For license see LICENSE.txt        */

#ifndef __BOX_ARCHIVER_FILESYSTEM_H__
  #define __BOX_ARCHIVER_FILESYSTEM_H__

  #include "types.h"
  #include "metadata.h"

  void ba_mkdir(char *path, ba_Entry *dir);
  void ba_load_fs_tree(char *path, ba_Entry **first_entry, fsize_t *data_size);		/* if *data_size isn't NULL, every time a file is added its size is added to this */
  ba_Meta* ba_get_meta(char *path);		/* Gets the metadata for the given file in the filesystem	*/
  fsize_t ba_fsize(char *path);

#endif
