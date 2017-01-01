
#ifndef __BOX_ARCHIVER_MAIN_H__
  #define __BOX_ARCHIVER_MAIN_H__

  #include "box_archive.h"

  #define BOX_ARCHIVER_VERSION "v0.07 Dev alpha"
  #define BOX_ARCHIVER_LICENSE "GPLv3"

  enum Job {
	  NONE,
	  CREATE,
	  EXTRACT,
	  LIST,
	  DETAILS,
	  GET_FORMAT,
	  PRINT_HEADER
  };

  void version();
  void help(char *progname);

  void print_opt_err(char optopt);
  void rec_list_func    (ba_Entry *first_entry);
  void rec_extract_func (BoxArchive *arch, ba_Entry *first_entry, char *dest);

#endif
