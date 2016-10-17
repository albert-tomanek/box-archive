#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "main.h"
#include "errors.h"
#include "box_archive.h"

int main (int argc, char *argv[])
{
	/* The archive struct */
	BoxArchive *archive = NULL;

	/* Strings that *might* be used */
	char arg;
	
	/* Switches */
	uint8_t debug = 0;

	enum Job job;

	if (argc < 2)
	{
		printf("For help, use the '-h' option.\n");
		return 0;
	}

	while ((arg = getopt(argc, argv, "df:vhTH")) != -1)
	{
	switch (arg)
	{
		case 'h':
			help();
			return 0;
		break;
		case 'v':
			version();
			return 0;
		break;

		/* Switches */
		case 'd':					/* debug */
			debug = 1;
		break;

		/* Compulsory args */
		case 'f':					/* File */
		{
			char *boxfile = optarg;
			archive = ba_open(boxfile);
			break;
		}

		/* Commands */
		case 'H':
			job = PRINT_HEADER;
		break;
		case 'T':
			job = GET_FORMAT;
		break;

		case '?':
			print_opt_err(optopt);
		break;
	}
	}
	
	/* Switches */
	ba_debug(archive, debug);
	
	/* Now do the specified job */
	
	switch (job)
	{
		case NONE:
		break;
		
		case CREATE:
		break;
		
		case EXTRACT:
		break;
		
		case GET_FORMAT:
			printf("Format version is %d.\n", ba_get_format(archive));
		break;
		
		case PRINT_HEADER:
		{
			char *header = ba_get_header(archive);		/* Returned string is on _heap_ */
			printf("%s\n", header);
			free(header);
		break;
		}
	}

	ba_close(archive);
	return 0;
}



void print_opt_err(char optopt)
{
	switch (optopt)
	{
	case 'f':
		error(ERR_CMDLINE, "[ERROR] Invalid '-f' argument. Use:\n        -f <file> \n");
	break;

	default:
		printf("[WARNING] '-%c' is an invalid option! Ignoring.\n", optopt);
	}
}

void version()
{
	printf("The BOX archiver,\n");
	printf("\t%s\n", BOX_ARCHIVER_VERSION);
}

void help()
{
	printf(" The BOX archiver.\n");
	printf(" \n");
	printf(" Format:\n");
	printf("   box [-dhv] [Argument params] -f <BOX archive>\n");
	printf(" \n");
	printf(" Arguments:\n");
	printf("   -H			Print the header XML of an archive.\n");
	printf("   -T			Print the Type/version of an archive.\n");
	printf(" \n");
	printf("   -d			Print debug information.\n");
	printf("   -v			Print the archiver's version.\n");
	printf("   -h			Print this help text.\n");
	printf(" \n");
	printf(" EzXML XML Parser:\n");
	printf("   Copyright (c) 2004-2006 Aaron Voisine <aaron@voisine.org>\n");
	printf(" The BOX archiver:\n");
	printf("   Copyright (c) 2016 Albert Tomanek <electron826@gmail.com>\n\n");
}

