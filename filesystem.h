
#ifndef __BOX_ARCHIVER_FILESYSTEM_H__
  #define __BOX_ARCHIVER_FILESYSTEM_H__

  #include "types.h"

  void ba_mkdir(char *path, ba_Entry *dir);
  void ba_load_fs_tree(char *path, ba_Entry **first_entry, fsize_t *data_size);		/* if *data_size isn't NULL, every time a file is added its size is added to this */
  fsize_t ba_fsize(char *file);

#endif
