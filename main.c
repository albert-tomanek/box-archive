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
	char *boxfile;

	/* On/Off settings, stored as uint8_t */
	uint8_t debug = 0;

	if (argc < 2)
	{
		printf("For help, use the '-h' option.\n");
		return 0;
	}

	while ((arg = getopt(argc, argv, "df:vh")) != -1)
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
		case 'f':
			boxfile = optarg;
			archive = ba_open(boxfile, debug);
		break;

		/* Commands */
		case 'H':
		break;

		case '?':
			print_opt_err(optopt);
		break;
	}
	}


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
	printf(" \n");
	printf("   -d			Print debug information.\n");
	printf("   -v			Print the archiver's version.\n");
	printf("   -h			Print this help text.\n");
	printf(" \n");
}

