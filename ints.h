#ifndef __INTS_H__
  #define __INTS_H__
  
  #include <stdint.h>
  
  uint16_t cvt8to16(uint8_t dataFirst, uint8_t dataSecond);
  uint8_t *cvt16to8(uint16_t dataAll);
  
#endif
