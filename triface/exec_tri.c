/*
 * Jonathan Sarba
 * 10 / 12 / 2003
 * 
 * set perm to 4750
 *
 * Uso: ./exec_bp [hostname] [instance] [copies]
*/
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

#define PATH	"/home/peb8/dev/sicom/dev/"
#define	PRG	PATH"/bin/triface.sh"

#define MAX	512
#define DEFAULT_COPIES	1

extern char **environ;
extern int errno;

int main(int argc, char **argv)
{
	char 	*prg[2] = { NULL };

	prg[0] = PRG;
	prg[1] = NULL;

	if( execve(prg[0], prg, environ) == -1 )
	{
		perror("execve()");
	}

	exit(0);
}

