/* Copyright (C) 2017  Albert Tomanek *
 * For license see LICENSE.txt        */

#include "byteorder.h"

#ifdef __unix__
	#include <arpa/inet.h>
	#include <netinet/in.h>		/* Unnecessary, but 'man htons' reccommends this. */

	uint16_t stol16(uint16_t system_endian)
	{
		/* stol16 = system to little endianness */

		return (uint16_t) htons((short) system_endian);
	}

	uint16_t ltos16(uint16_t little_endian)
	{
		/* ltos16 = little to system endian */

		return (uint16_t) ntohs((short) little_endian);
	}

#endif
