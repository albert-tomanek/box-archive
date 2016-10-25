/*
 * Just a simple linked list of file objects,
 * passed from the ba library to the program using it.
 */

#ifndef __BOX_ARCHIVE_FILELIST_H__
  #define __BOX_ARCHIVE_FILELIST_H__
  
  #include "file.h"
  
  struct ba_FileList {
      struct ba_File     *file;
      struct ba_FileList *next;
  };
  
  typedef struct ba_FileList ba_FileList;
  
  void bafl_add(ba_FileList **first_file, ba_File *file);
  void bafl_free(ba_FileList **first_file);
  
#endif
