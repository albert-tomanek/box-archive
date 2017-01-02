#ifndef __BOX_ARCHIVE_BYTEORDER_H__
  #define __BOX_ARCHIVE_BYTEORDER_H_

  #include <stdint.h>

  uint16_t stol16(uint16_t little_endian);		/* System to little */
  uint16_t ltos16(uint16_t little_endian);		/* Little to system */

#endif
