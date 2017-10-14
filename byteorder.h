/* Copyright (C) 2017  Albert Tomanek *
 * For license see LICENSE.txt        */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __BOX_ARCHIVE_BYTEORDER_H__
  #define __BOX_ARCHIVE_BYTEORDER_H_

  #include <stdint.h>

  uint16_t stol16(uint16_t little_endian);		/* System to little */
  uint16_t ltos16(uint16_t little_endian);		/* Little to system */

#endif

#ifdef __cplusplus
}
#endif
