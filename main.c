/* Copyright (C) 2017  Albert Tomanek *
 * For license see LICENSE.txt        */

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
    char *dest = NULL;    /* Where to extract/move to */
    char *src  = NULL;    /* Where to get files from */
    char *path = NULL;    /* The path of a single file within the archive */
    char *boxfile = NULL;    /* Source archive */
    char *outfile = NULL;    /* Archive to save to */
    char *start_entry_path= NULL;
    ba_Entry *start_entry = NULL;    /* This is the entry which we work with. Unless the '-F' flag is used, this will be archive->entry_tree. */
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

    while ((arg = getopt(argc, argv, "lc:x:f:o:F:drm:TH")) != -1)
    {
    switch (arg)
    {
        case 'f':                    /* File */
        {
            boxfile = strdup(optarg);
            break;
        }
        case 'F':                    /* Used to specify the entry */
        {
            start_entry_path = strdup(optarg);
            break;
        }
        case 'o':
        {
            outfile = strdup(optarg);
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
            dest = strdup(optarg);    /* Store the location of where to extract to */
            job = EXTRACT;

            break;
        }
        case 'r':
        {
            job = REMOVE;

            break;
        }

        case 'm':
        {
            job = MOVE;

            dest = strdup(optarg);

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

    /* Set the source archive to be overwritten    *
     * if no outfile specified.                    */
    if (! outfile)
    {
        if (boxfile)
            outfile = strdup(boxfile);
    }

    /* Intialise the start_entry pointer */
    if (start_entry_path)
    {
        ba_Entry *start_entry_orig = ba_get(archive, start_entry_path);
        check(start_entry_orig != NULL, "Entry not found.")

        /* Sorry, not the nicest way to do things... */

        start_entry = malloc(sizeof(ba_Entry));
        memcpy(start_entry, start_entry_orig, sizeof(ba_Entry));
        start_entry->next = NULL;                                    /* We set ->next to NULL, to prevent jobs being done simply on a branch of the tree, but on an entry and it's children only. */
    }
    else
    {
        start_entry = ba_get_entries(archive);    /* This just gives us archive->entry_tree */
    }

    /* Now do the specified job */

    switch (job)
    {
        case CREATE:
        {
            if (! boxfile) { fprintf(stderr, "Use the -f argument to specify which .box file you want to save to.\n"); break; }

            /* Hopefully the API will provide a nicer way    *
             * to do this in the future...                    */

            ba_load_fs_tree(src, &(archive->entry_tree), &(archive->__data_size));

            ba_save(archive, outfile);

            break;
        }

        case EXTRACT:
        {
            if (! boxfile) { fprintf(stderr, "Use the -f argument to specify which .box file you want to extract.\n"); break; }
            if (! archive) break;

            ba_Entry *first_entry = start_entry;
            check(first_entry, "Error getting entries.");

            rec_extract_func(archive, first_entry, dest);    /* Run our recursive function which goes through each direcotry            *
                                                              * and extracts the files in it. Note: 'dest' is defined beforehand.    */

            break;
        }

        case REMOVE:
        {
            if (! boxfile)          { fprintf(stderr, "Use the -f argument to specify which .box file you want to extract.\n"); break; }
            if (! start_entry_path) { fprintf(stderr, "Use the -F argument to specify which file or directory you want to remove.\n"); break; }
            if (! archive) break;

            ba_Entry *rm_entry = ba_get(archive, start_entry_path);
            if     (! rm_entry) { fprintf(stderr, "Entry not found.\n"); break; }
            ba_remove(archive, &rm_entry);

            ba_save(archive, outfile);

            break;
        }

        case MOVE:
        {
            if (! boxfile)          { fprintf(stderr, "Use the -f argument to specify which .box file you want to extract.\n"); break; }
            if (! start_entry_path) { fprintf(stderr, "Use the -F argument to specify which file or directory you want to remove.\n"); break; }
            if (! archive) break;

            ba_Entry *mv_entry   = ba_get(archive, start_entry_path);
            ba_Entry *dest_entry = ba_get(archive, dest);
            if     (! mv_entry)   { fprintf(stderr, "Entry '%s' not found.\n", start_entry_path); break; }
            if     (! dest_entry) { fprintf(stderr, "Entry '%s' not found.\n", dest); break; }
            ba_move(archive, mv_entry, &dest_entry);

            ba_save(archive, outfile);

        }

        case LIST:
        {
            if (! boxfile) { fprintf(stderr, "Use the -f argument to specify which .box file you want to use.\n"); break; }
            if (! archive) break;

            ba_Entry *first_entry = start_entry;

            check(first_entry, "Error getting entries.");

            rec_list_func(first_entry);

            break;
        }

        case DETAILS:
        {
            if (! boxfile) { fprintf(stderr, "Use the -f argument to specify which .box file you want to use.\n"); break; }
            ba_Entry *entry = ba_get(archive, start_entry_path);

            if (! entry) { fprintf(stderr, "Entry not found.\n"); break; }

			/* These are designed to be decently machine-readable:	*
			 * <name>:\t<value>\n									*/

            printf("Type:\t%s\n", ba_entry_nice_type(entry->type) );
            printf("Name:\t%s\n", entry->name);
            printf("Path:\t%s\n", entry->path);

            if (entry->type == ba_EntryType_FILE && entry->file_data != NULL)
            {
                printf("Start position:\t%llu\n", (long long unsigned) entry->file_data->__start);
                printf("Size (bytes):\t%llu\n",   (long long unsigned) entry->file_data->__size);
            }

            if (entry->type == ba_EntryType_DIR  && entry->child_entries != NULL)
            {
                printf("Size (bytes):\t%llu\n", (long long unsigned) ba_treesize(entry));
                printf("Contents:\t");
                for (ba_Entry *current = entry->child_entries; current != NULL; current = current->next)
                {
                    printf("%s\t", current->name);
                }
                printf("\n");
            }

			if (entry->meta != NULL)
			{
				char atime_str[BOX_ARCHIVER_STRLEN];
				char mtime_str[BOX_ARCHIVER_STRLEN];
				struct tm atime_tm;
				struct tm mtime_tm;

				localtime_r(&(entry->meta->atime), &atime_tm);
				localtime_r(&(entry->meta->mtime), &mtime_tm);

				strftime(atime_str, BOX_ARCHIVER_STRLEN, "%Y-%m-%d %H:%M:%S", &atime_tm);
				strftime(mtime_str, BOX_ARCHIVER_STRLEN, "%Y-%m-%d %H:%M:%S", &mtime_tm);

				printf("Last accessed:\t%s\n", atime_str);
				printf("Last modified:\t%s\n", mtime_str);
			}

            break;
        }

        case GET_FORMAT:
        {
            if (! boxfile) { fprintf(stderr, "Use the -f argument to specify which .box file you want to use.\n"); break; }
            if (! archive) break;

            printf("Format version is %d.\n", ba_get_format(archive));

            break;
        }

        case PRINT_HEADER:
        {
            if (! boxfile) { fprintf(stderr, "Use the -f argument to specify which .box file you want to use.\n"); break; }
            if (! archive) break;

            char *header = ba_get_header(archive);        /* Returned string is on _heap_ */
            printf("%s\n", header);
            free(header);

            break;
        }

        case NONE:
        default:
        break;
    }

    if (archive)    ba_close(archive);

    if (dest)        free(dest);
    if (src)        free(src);
    if (path)        free(path);
    if (start_entry_path) free(start_entry_path);		/* We don't free start_entry, because that's already been freed by ba_close(). */
    if (outfile)    free(outfile);
    if (boxfile)    free(boxfile);

    return 0;

error:
    if (archive)    ba_close(archive);

    if (dest)        free(dest);
    if (src)        free(src);
    if (path)        free(path);
    if (start_entry_path) free(start_entry_path);
    if (outfile)    free(outfile);
    if (boxfile)    free(boxfile);

    return 1;
}

void rec_list_func (ba_Entry *first_entry)
{
    if (! first_entry)    return;

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
    if (! first_entry)    return;

    ba_Entry *current = first_entry;

    char *out_name;

    while (current)
    {
        out_name = dupcat(dest, (dest[strlen(dest)-1] == BA_SEP[0] ? "" : BA_SEP), current->path, "");        /* The tertiary operator is used to avoid things like extracting to '/tmp//test.txt' when extracting to '/tmp/' */

        printf("%s\n", out_name);

        if (current->type == ba_EntryType_FILE)
        {
            ba_extract(arch, current, out_name);
        }
        else if (current->type == ba_EntryType_DIR)
        {
            ba_mkdir(dest, current);            /* Make the direcotry */
            rec_extract_func(arch, current->child_entries, dest);
        }


        free(out_name);            /* It's on heap, because dupcat uses malloc */
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
    printf("The BOX archiver\n");
    printf("%s\n", BOX_ARCHIVER_VERSION);
}

void help(char *progname)
{
    printf(" The BOX archiver.\n");
    printf(" \n");
    printf(" (Note: 'entry' means file OR directory.)\n");
    printf(" \n");
    printf(" Format:\n");
    printf("   %s [-hvl] [Argument params] -f <BOX archive>\n", progname);
    printf(" \n");
    printf(" Arguments:\n");
    printf("   -H           Print the header XML of an archive.\n");
    printf("   -T           Print the Type/version of an archive.\n");
    printf("   -l           List the files in the archive.\n");
    printf("   -f <arch>    Use this archive file.\n");
    printf("   -x <dest>    Extract the files to the given destination.\n");
    printf("   -c <src>     Create an archive from the given directory.\n");
    printf("   -o <out>     Save archive to <out> instead of overwriting\n");
    printf("                source file specified by -f.\n");
    printf("   -r           Remove the entry specified by -F.\n");
    printf("   -m <dest>    Move the entry specified by -F to <dest>.\n");
    printf("   -d           Show a file's details (file specified by -F).\n");
    printf("   -F <path>    Operate on THIS entry.\n");
    printf(" \n");
    printf("   -v           Print the archiver's version.\n");
    printf("   -h           Print this help text.\n");
    printf(" \n");
    printf(" EzXML XML Parser:\n");
    printf("   Copyright (c) 2004-2006 Aaron Voisine <aaron@voisine.org>\n");
    printf(" The BOX archiver:\n");
    printf("   Copyright (c) 2016-2017 Albert Tomanek <electron826@gmail.com>\n");
    printf("\n");
}
