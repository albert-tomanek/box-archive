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
	char *dest = NULL;	/* Where to extract to */
	char *src  = NULL;	/* Where to get files from */
	char *path = NULL;	/* The path of a single file within the archive */
	char *boxfile = NULL;
	char *start_entry_path= NULL;
	ba_Entry *start_entry = NULL;	/* This is the entry which we work with. Unless the '-F' flag is used, this will be archive->entry_list. */
	enum Job job;

	if (argc < 2)
	{
		printf("For help, use the '-h' option.\n");
		return 0;
	}

	if (argc == 2)
	{
		if (! strcmp(argv[1], "-h") || ! strcmp(argv[1], "--help"))
		{
			help(argv[0]);
		}
		else if (! strcmp(argv[1], "-v") || ! strcmp(argv[1], "--version"))
		{
			version();
		}

		return 0;
	}

	while ((arg = getopt(argc, argv, "lc:x:f:F:d:TH")) != -1)
	{
	switch (arg)
	{
		/* Compulsory args */
		case 'f':					/* File */
		{
			boxfile = strdup(optarg);
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
		case 'd':
		{
			path = strdup(optarg);
			job  = DETAILS;

			break;
		}
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
		case 'F':
		{
			start_entry_path = strdup(optarg);
			break;
		}

		case '?':
		default:
			print_opt_err(arg);
		break;
	}
	}

	/* Create an archive if it has not been created yet */
	if (! archive)
	{
		if (job == CREATE)
		{
			archive = ba_new();
		}
		else
		{
			archive = ba_open(boxfile);
		}
	}

	/* Intialise the start_entry pointer */
	if (start_entry_path)
	{
		ba_Entry *start_entry_orig = ba_get(archive, start_entry_path);
		if (! start_entry_orig)	fprintf(stderr, "Entry not found.\n");

		/* Sorry, not the nicest way to do things... */

		start_entry = malloc(sizeof(ba_Entry));
		memcpy(start_entry, start_entry_orig, sizeof(ba_Entry));
		start_entry->next = NULL;									/* We set ->next to NULL, to prevent jobs being done simply on a branch of the tree, but on an entry and it's children only. */
	}
	else
	{
		start_entry = ba_get_entries(archive);	/* This just gives us archive->entry_list */
	}

	/* Now do the specified job */

	switch (job)
	{
		case CREATE:
		{
			/* Hopefully the API will provide a nicer way	*
			 * to do this in the future...					*/

			ba_load_fs_tree(src, &(archive->entry_list));

			ba_save(archive, boxfile);

			break;
		}

		case EXTRACT:
		{
			if (! archive) break;

			ba_Entry *first_entry = start_entry;
			check(first_entry, "Error getting entries.");

			rec_extract_func(archive, first_entry, dest);	/* Run our recursive function which goes through each direcotry			*
													 		 * and extracts the files in it. Note: 'dest' is defined beforehand.	*/

			break;
		}

		case LIST:
		{
			if (! archive) break;

			ba_Entry *first_entry = start_entry;

			check(first_entry, "Error getting entries.");

			rec_list_func(first_entry);

			break;
		}

		case DETAILS:
		{
			ba_Entry *entry = ba_get(archive, path);

			if (! entry)
			{
				fprintf(stderr, "Entry not found.\n");
				break;
			}

			printf("Type:\t%s\n", ba_entry_nice_type(entry->type) );
			printf("Name:\t%s\n", entry->name);
			printf("Path:\t%s\n", entry->path);

			printf("\n");

			if (entry->type == ba_EntryType_FILE && entry->file_data != NULL)
			{
				printf("Start position:\t%llu\n", (long long unsigned) entry->file_data->__start);
				printf("Size (bytes):\t%llu\n",   (long long unsigned) entry->file_data->__size);

				printf("\n");
			}

			if (entry->type == ba_EntryType_DIR  && entry->child_entries != NULL)
			{
				printf("Contents:\t");
				for (ba_Entry *current = entry->child_entries; current != NULL; current = current->next)
				{
					printf("%s\t", current->name);
				}
				printf("\n");
			}

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

		case NONE:
		default:
		break;
	}

	if (archive)	ba_close(archive);

	if (dest)		free(dest);
	if (src)		free(src);
	if (path)		free(path);
	if (boxfile)	free(boxfile);

	return 0;

error:
	if (archive)	ba_close(archive);

	if (dest)		free(dest);
	if (src)		free(src);
	if (path)		free(path);
	if (boxfile)	free(boxfile);

	return 1;
}

void rec_list_func (ba_Entry *first_entry)
{
	if (! first_entry)	return;

	ba_Entry *current = first_entry;

	while (current)
	{
		printf("%s\n", current->path);

		if (current->type == ba_EntryType_DIR)
		{
			rec_list_func(current->child_entries);
		}

		current = current->next;
	}
}

void rec_extract_func (BoxArchive *arch, ba_Entry *first_entry, char *dest)
{
	if (! first_entry)	return;

	ba_Entry *current = first_entry;

	char *out_name;

	while (current)
	{
		out_name = dupcat(dest, (dest[strlen(dest)-1] == BA_SEP[0] ? "" : BA_SEP), current->path, "");		/* The tertiary operator is used to avoid things like extracting to '/tmp//test.txt' when extracting to '/tmp/' */

		printf("%s\n", out_name);

		if (current->type == ba_EntryType_FILE)
		{
			ba_extract(arch, current, out_name);
		}
		else if (current->type == ba_EntryType_DIR)
		{
			ba_mkdir(dest, current);			/* Make the direcotry */
			rec_extract_func(arch, current->child_entries, dest);
		}


		free(out_name);			/* It's on heap, because dupcat uses malloc */
		current = current->next;
	}
}

void print_opt_err(char optopt)
{
	switch (optopt)
	{
	case 'f':
		fprintf(stderr, "[ERROR] Invalid '-f' argument. Use: -f <file> \n");
	break;
	case 'F':
		fprintf(stderr, "[ERROR] Invalid '-F' argument. Use: -F <path in archive> \n");
	break;
	case 'd':
		fprintf(stderr, "[ERROR] Invalid '-d' argument. Use: -d <path in archive> \n");
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

void help(char *progname)
{
	printf(" The BOX archiver.\n");
	printf(" \n");
	printf(" Format:\n");
	printf("   %s [-hvl] [Argument params] -f <BOX archive>\n", progname);
	printf(" \n");
	printf(" Arguments:\n");
	printf("   -H			Print the header XML of an archive.\n");
	printf("   -T			Print the Type/version of an archive.\n");
	printf("   -l			List the files in the archive.\n");
	printf("   -f <arch>	Use this archive file.\n");
	printf("   -x <dest>	Extract the files to the given destination.\n");
	printf("   -c <src>		Create an archive from the given directory.\n");
	printf("   -d <path>	Show a file's details.\n");
	printf("   -F <path>	Work only with this file in the archive.\n");
	printf(" \n");
	printf("   -v			Print the archiver's version.\n");
	printf("   -h			Print this help text.\n");
	printf(" \n");
	printf(" EzXML XML Parser:\n");
	printf("   Copyright (c) 2004-2006 Aaron Voisine <aaron@voisine.org>\n");
	printf(" The BOX archiver:\n");
	printf("   Copyright (c) 2016 Albert Tomanek <electron826@gmail.com>\n\n");
}
