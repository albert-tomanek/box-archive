#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "main.h"

int main (int argc, char *argv[])
{
	/* Strings that *might* be used */
	char arg;
	char *boxfile;

	/* On/Off settings, stored as uint8_t */
	uint8_t debug = 0
			;

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

		/* Compulsory args */
		case 'f':
			boxfile = optarg;
			if (debug)
				printf("[DEBUG] boxfile = %s\n", boxfile);
		break;

		/* Switches */
		case 'd':					/* debug */
			debug = 1;
		break;

		case '?':
			print_opt_err(optopt);
		break;
	}
	}


	return 0;
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
	printf("   -d			Print debug information.\n");
	printf("   -v			Print the archiver's version.\n");
	printf("   -h			Print this help text.\n");
	printf(" \n");
}

void print_opt_err(char optopt)
{
	switch (optopt)
	{
	case 'f':
		printf("[ERROR] Invalid '-f' argument. Use:\n");
		printf("        -f <file> \n");
		exit(ERR_CMDLINE);
	break;

	default:
		printf("[WARNING] '-%c' is an invalid option! Ignoring.\n", optopt);
	}
}
