/* A struct that contains metadata about an entry.	*
 * This metadata must be able to apply to both 		*
 * files and directories.							*/

#ifndef __BOX_ARCHIVE_META_H__
  #define __BOX_ARCHIVE_META_H__

  #include <time.h>

  struct ba_Meta
  {
	  time_t atime;		/* Last access time	*/
	  time_t mtime;		/* Last modified time	*/
  };

  typedef struct ba_Meta ba_Meta;

  void ba_meta_free(ba_Meta *meta);

#endif
