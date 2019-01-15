#include "arg.h"

void usage(char *prgname)
{
	printf("%s %s (c) 2005 by %s\n\nUsage: %s [options] [hostname] [filename]\n",
		PROGRAM, VERSION, AUTHOR, prgname);
  	printf("Options:\n"
    "  -l user : log in as user\n"
    "  -p port : connect to port\n");

  exit(-1);
}

char *GetLastArg(int argc, char **argv)
{
	static int index = 0;
	int opt;
	while ( (opt=getopt(argc, argv, "")) != -1 )
	{
		switch(opt)
		{
			default:
				usage(argv[0]);
		}
	}
	if ( optind < argc )
		return argv[(optind++)+(index++)];
	else 
		return NULL;
}
