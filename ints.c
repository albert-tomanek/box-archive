#include <stdint.h>
#include <stdlib.h>
#include "ints.h"

/* Used to convert two 8-bit values to a 16-bit value (i.e. the header data in the rso file) */
uint16_t cvt8to16(uint8_t dataFirst, uint8_t dataSecond)
{
	uint16_t dataBoth = 0x0000;

	dataBoth = dataFirst;
	dataBoth = dataBoth << 8;
	dataBoth |= dataSecond;

	return dataBoth;
}

uint8_t *cvt16to8(uint16_t dataAll)
{
	uint8_t *arrayData = malloc(sizeof(uint8_t) * 2);

	arrayData[0] = (dataAll >> 8) & 0x00FF;
	arrayData[1] = dataAll & 0x00FF;

	return arrayData;
}
