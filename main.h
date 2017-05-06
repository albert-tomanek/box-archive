/* Copyright (C) 2017  Albert Tomanek *
 * For license see LICENSE.txt        */

#ifndef __BOX_ARCHIVER_MAIN_H__
  #define __BOX_ARCHIVER_MAIN_H__

  #include "box_archive.h"

  #define BOX_ARCHIVER_VERSION "development alpha"
  #define BOX_ARCHIVER_LICENSE "LGPLv2.1"
  #define BOX_ARCHIVER_STRLEN 64	/* Length of strings used to store atimes and mtimes before printing them. */

  enum Job {
	  NONE = 0,
	  CREATE,
	  EXTRACT,
	  REMOVE,
	  MOVE,
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
