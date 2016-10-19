#ifndef __BOX_ARCHIVER_MAIN_H__
  #define __BOX_ARCHIVER_MAIN_H__
  
  #include "box_archive.h"
  
  /*build*/

  #define BOX_ARCHIVER_VERSION "v0.01 Dev Alpha"
  #define BOX_ARCHIVER_LICENSE "GPL"
  
  enum Job {
	  NONE,
	  CREATE,
	  EXTRACT,
	  LIST,
	  GET_FORMAT,
	  PRINT_HEADER
  };
  
  void version();
  void help();
  
  void print_opt_err(char optopt);

#endif
