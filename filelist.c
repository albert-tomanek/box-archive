#include <string.h>
#include <stdlib.h>

#include "errors.h"
#include "file.h"
#include "filelist.h"

/* Private stuff */

ba_FileList* __bafl_getlast(ba_FileList *first);

/*  */

void bafl_add(ba_FileList **first_file, ba_File *file)
{
	ba_FileList *new_file = malloc(sizeof(ba_FileList));
	
	new_file->next = NULL;
	new_file->file = file;
	
	if (*first_file == NULL)
	{
		//printf("Adding '%s' as first line to list at %p\n", file_path, *first_file);
		/* If the list is empty */
		
		*first_file = new_file;
		
		//printf("first_file = %p\n", first_file);
	}
	else
	{
		/* Add it to the end */
		__bafl_getlast(*first_file)->next = new_file;
	}
}

void bafl_free(ba_FileList **first_file)	/* Double-pointer because we will be changing the original pointer to NULL */
{
	ba_FileList *current_file = *first_file;
	ba_FileList *next;

	if (! current_file)		/* In case first_file is NULL or we are on the last file */
		return;
	
	while(current_file)
	{
		next = current_file->next;
		
		ba_file_free(current_file->file);	/* Free the ba_File struct that it points to. */
		free		(current_file);
		
		current_file = next;
	}

	*first_file = NULL;		/* Change it to null since the current pointer would be invalid */
	
	return;
}

ba_FileList* __bafl_getlast(ba_FileList *first)
{
	
	if (first == NULL)
	{
		error(ERR_NULLPTR, "[ERROR] Null pointer passed to __bafl_getlast() for ba_FileList *first.\n");
	}
	
	ba_FileList *current = first;
	
	while (current->next)
	{
		current = current->next;
	}
	
	return current;
}