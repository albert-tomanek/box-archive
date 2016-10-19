/*
 * Just a simple linked list of file names,
 * passed from the ba library to the program using it.
 */

#ifndef __BOX_ARCHIVE_FILELIST_H__
  #define __BOX_ARCHIVE_FILELIST_H__
  
  struct ba_FileList {
      char  *path;
      struct ba_FileList *next;
  };
  
  typedef struct ba_FileList ba_FileList;
  
  void bafl_add(ba_FileList **first_file, char *file_path);
  void bafl_free(ba_FileList **first_file);
  
#endif
