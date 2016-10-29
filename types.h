/* Internal type for values like ba_File->__start ,
 * so that the value can be changed with future compiles.
 */

#ifndef __BOX_ARCHIVE_SIZES_H__
  #define __BOX_ARCHIVE_SIZES_H__
  
  #include <stdint.h>
  
  #define fsize_t	uint64_t
  #define offset_t 	uint64_t
  #define hdrlen_t 	uint16_t
  
#endif