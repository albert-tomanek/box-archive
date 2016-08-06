#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "main.h"

int main (int argc, char *argv[])
{
	char arg;

	if (argc < 2)
	{
		printf("For help, use the '-h' option.\n");
		return 0;
	}

	while ((arg = getopt(argc, argv, "vh")) != -1)
	{
	switch (arg)
	{
		case 'h':
			help();
		break;
		case 'v':
			version();
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
	printf("The BOX archiver.\n");
	printf("\n");
	printf("Arguments:\n");
	printf(" -v			Print the archiver's version.\n");
	printf(" -h			Print this help text.\n");
	printf("\n");
}
