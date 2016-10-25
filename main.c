#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "dbg.h"

#include "main.h"
#include "box_archive.h"

int main (int argc, char *argv[])
{
	/* The archive struct */
	BoxArchive *archive = NULL;

	char arg;

	enum Job job;

	if (argc < 2)
	{
		printf("For help, use the '-h' option.\n");
		return 0;
	}

	while ((arg = getopt(argc, argv, "dlf:vhTH")) != -1)
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
		case 'l':
			job = LIST;
		break;

		case '?':
			print_opt_err(optopt);
		break;
	}
	}
	
	/* Now do the specified job */
	
	switch (job)
	{
		case NONE:
		break;
		
		case CREATE:
		break;
		
		case EXTRACT:
		break;
		
		case LIST:
		{
			ba_FileList *file = ba_getfiles(archive);	// more like file*s*
			
			check(file, "Error getting files.")
			
			while (file)
			{
				printf("%s\n", file->file->path);
				
				file = file->next;
			}
			
			bafl_free(&file);
			
			break;
		}
		
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

error:
	return 1;
}



void print_opt_err(char optopt)
{
	switch (optopt)
	{
	case 'f':
		fprintf(stderr, "[ERROR] Invalid '-f' argument. Use: -f <file> \n");
	break;

	default:
		fprintf(stderr, "[WARNING] '-%c' is an invalid option! Ignoring.\n", optopt);
	}
	
	exit(1);
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

