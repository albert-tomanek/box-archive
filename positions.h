#ifndef __BOX_ARCHIVER_POSITIONS_H__
  #define __BOX_ARCHIVER_POSITIONS_H__
  
  #define P_MAGIC_NO		0
  #define P_HEADER_LENGTH	4		/* The offset at which the header length value is stored */
  #define P_HEADER			7		/* Includes the two-byte length number */
  #define P_FILE_DATA		9		/* Don't forget to add the length of the XML header to this! */
  
#endif