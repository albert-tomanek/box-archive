/* Internal type for values like ba_File->__start ,
 * so that the value can be changed with future compiles.
 */

#ifndef __BOX_ARCHIVE_SIZES_H__
  #define __BOX_ARCHIVE_SIZES_H__
  
  #include <stdint.h>
  
  #define offset_t 	uint32_t
  #define hdrlen_t 	uint16_t
  
#endif