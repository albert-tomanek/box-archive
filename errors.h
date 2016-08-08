#ifndef __BOX_ARCHIVER_ERRORS_H__
  #define __BOX_ARCHIVER_ERRORS_H__
  
  #include <stdlib.h>
  #include <stdio.h>
  
  #define error(C, S, ...) fprintf(stderr, S, ##__VA_ARGS__); exit(C);
  
  /* Error codes */
  #define ERR_NONE		0
  #define ERR_CMDLINE	1
  #define ERR_FFORMAT	2   /* File format error */
  #define ERR_MEM		3	/* Memory error */
  #define ERR_FOPEN		4
  
  #define ERR_OTHER		255
  
#endif
