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

	char  arg;
	char *dest = NULL;

	enum Job job;

	if (argc < 2)
	{
		printf("For help, use the '-h' option.\n");
		return 0;
	}

	while ((arg = getopt(argc, argv, "dlx:f:vhTH")) != -1)
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
		case 'x':
		{
			dest = strdup(optarg);	/* Store the location of where to extract to */
			job = EXTRACT;
		}
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
		{
			if (! archive) break;
			
			ba_FileList *file = ba_getfiles(archive);
			check(file, "Error getting files.");
			
			char *out_name;
			
			while (file)
			{
				out_name = dupcat(dest, BA_SEP, file->file->path);
				
				printf("%s\n", out_name);
				ba_extract(archive, file->file->path, out_name);
				
				file = file->next;
			}
			
			break;
		}
		
		case LIST:
		{
			if (! archive) break;
			
			ba_FileList *file = ba_getfiles(archive);	// more like file*s*
			
			check(file, "Error getting files.");
			
			while (file)
			{
				printf("%s\n", file->file->path);
				
				file = file->next;
			}
			
			bafl_free(&file);
			
			break;
		}
		
		case GET_FORMAT:
		{
			if (! archive) break;
			
			printf("Format version is %d.\n", ba_get_format(archive));
			
			break;
		}
		
		case PRINT_HEADER:
		{
			if (! archive) break;
			
			char *header = ba_get_header(archive);		/* Returned string is on _heap_ */
			printf("%s\n", header);
			free(header);
			break;
		}
	}

	if (archive)	ba_close(archive);
	if (dest)		free(dest);
	
	return 0;

error:
	if (archive)	ba_close(archive);
	if (dest)		free(dest);
	
	return 1;
}



void print_opt_err(char optopt)
{
	switch (optopt)
	{
	case 'f':
		fprintf(stderr, "[ERROR] Invalid '-f' argument. Use: -f <file> \n");
	break;
	case 'x':
		fprintf(stderr, "[ERROR] Invalid '-x' argument. Use: -x <destination> \n");
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
	printf("   box [-hvl] [Argument params] -f <BOX archive>\n");
	printf(" \n");
	printf(" Arguments:\n");
	printf("   -H			Print the header XML of an archive.\n");
	printf("   -T			Print the Type/version of an archive.\n");
	printf("   -l			List the files in the archive.\n");
	printf("   -x <dest>	Extract the files to the given destination.\n");
	printf(" \n");
	printf("   -v			Print the archiver's version.\n");
	printf("   -h			Print this help text.\n");
	printf(" \n");
	printf(" EzXML XML Parser:\n");
	printf("   Copyright (c) 2004-2006 Aaron Voisine <aaron@voisine.org>\n");
	printf(" The BOX archiver:\n");
	printf("   Copyright (c) 2016 Albert Tomanek <electron826@gmail.com>\n\n");
}

char* dupcat(char *str1, char *str2, char *str3)
{
	/* Like strcat, but makes a copy of the text */
	char *out;
	
	out = calloc( strlen(str1)+strlen(str2)+strlen(str3)+1, sizeof(char));
	sprintf(out, "%s%s%s", str1, str2, str3);
	
	return out;
}
