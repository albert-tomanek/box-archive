#include <stdlib.h>
#include "entry.h"
#include <stdio.h>

void ba_file_free(struct ba_File **file)
{
    if (! file)
            return;

    if ((*file)->buffer)    free((*file)->buffer);
    free(*file);

    *file = NULL;
    return;
}
