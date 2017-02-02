/* Internal type for values like ba_File->__start ,
 * so that the value can be changed with future compiles.
 */

#ifndef __BOX_ARCHIVE_SIZES_H__
  #define __BOX_ARCHIVE_SIZES_H__

  #include <stdint.h>

  /* These are signed (therefore going only to 9,223,372,036,854,775,807 bytes),	*
   * so that it can be signalled that they have not been set yet by being set to -1	*/

  #define fsize_t	int64_t
  #define offset_t 	int64_t
  #define hdrlen_t 	int16_t

#endif
