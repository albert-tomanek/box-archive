#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "dbg.h"
#include "dupcat.h"

#include "main.h"
#include "box_archive.h"
#include "filesystem.h"

int main (int argc, char *argv[])
{
	/* The archive struct */
	BoxArchive *archive = NULL;

	char  arg;
	char *dest = NULL;	/* Where to extract from */
	char *src  = NULL;	/* Where to get files from */

	enum Job job;

	if (argc < 2)
	{
		printf("For help, use the '-h' option.\n");
		return 0;
	}

	while ((arg = getopt(argc, argv, "dlc:x:f:vhTH")) != -1)
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
		case 'c':
		{
			src = strdup(optarg);
			job = CREATE;

			break;
		}
		case 'x':
		{
			dest = strdup(optarg);	/* Store the location of where to extract to */
			job = EXTRACT;

			break;
		}

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
		{
			archive = ba_new();
			check(archive, "Error creating new archive.");

			ba_EntryList *entry = NULL;

			entry = ba_getdir(src);

			while (entry)
			{
				if (entry->entry->type == ba_EntryType_FILE)
				{
					/* Add the current file to the archve */
					ba_add(archive, entry->entry);
				}

				entry = entry->next;
			}

			break;
		}

		case EXTRACT:
		{
			if (! archive) break;

			ba_EntryList *entry = ba_get_entries(archive);
			check(entry, "Error getting entries.");

			char *out_name;

			while (entry)
			{
				out_name = dupcat(dest, (dest[strlen(dest)-1] == BA_SEP[0] ? "" : BA_SEP), entry->entry->path);		/* The tertiary operator is used to avoid things like extracting to '/tmp//test.txt' when extracting to '/tmp/' */

				printf("%s\n", out_name);

				if (entry->entry->type == ba_EntryType_FILE)
				{
					ba_extract(archive, entry->entry->path, out_name);
				}
				else if (entry->entry->type == ba_EntryType_DIR)
				{
					ba_mkdir(dest, entry->entry);
				}

				entry = entry->next;
			}

			break;
		}

		case LIST:
		{
			if (! archive) break;

			ba_EntryList *entry = ba_get_entries(archive);	// more like file*s*

			check(entry, "Error getting entries.");

			while (entry)
			{
				printf("%s\n", entry->entry->path);

				entry = entry->next;
			}

			bael_free(&entry);

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

void process_dir()
{
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
	case 'c':
		fprintf(stderr, "[ERROR] Invalid '-c' argument. Use: -c <source> \n");
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
	printf("   -c <src>		Create an archive from the given directory.\n");
	printf(" \n");
	printf("   -v			Print the archiver's version.\n");
	printf("   -h			Print this help text.\n");
	printf(" \n");
	printf(" EzXML XML Parser:\n");
	printf("   Copyright (c) 2004-2006 Aaron Voisine <aaron@voisine.org>\n");
	printf(" The BOX archiver:\n");
	printf("   Copyright (c) 2016 Albert Tomanek <electron826@gmail.com>\n\n");
}
