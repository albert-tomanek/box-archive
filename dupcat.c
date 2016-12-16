#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dupcat.h"

char* dupcat(char *str1, char *str2, char *str3, char *str4)
{
	/* Like strcat, but makes a copy of the text */
	char *out;

	out = calloc( strlen(str1) + strlen(str2) + strlen(str3) + strlen(str4) + 1, sizeof(char));
	sprintf(out, "%s%s%s%s", str1, str2, str3, str4);

	return out;
}
