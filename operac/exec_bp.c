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

#define PATH	"/usr/local/backup/bin"
#define	PRG	PATH"/backup.ksh"

#define	HOSTNAME	"-h%s"
#define INSTANCE	"-i%s"
#define COPIES		"-c%ld"

#define MAX	512
#define DEFAULT_COPIES	1

extern char **environ;

char *FmtPrepare(size_t sz, const char *fmt, ...)
{
	va_list	ap;
	char *p;

	if ( sz > MAX ) sz = MAX;

	if ( (p=(char *)calloc(sz, sizeof(char))) == NULL )
		return NULL;

	va_start(ap, fmt);
	vsprintf(p, fmt, ap);
	va_end(ap);

	return p;
}

	
int main(int argc, char **argv)
{
	size_t	copies, size;
	uid_t	UserId;
	char 	*prg[4] = { NULL };

	UserId = geteuid();

	if ( argc < 4 )
	{
		printf("use: %s [hostname] [instance] [copies]\n", argv[0]);
		exit(-1);
	}
	
	if ( UserId != 0 )
	{
	        printf("File must be setuid for root owner\n");
                exit(-1);	
	}
	else
	{
		prg[0] = PRG;

		if ( strtol(argv[3], NULL, 0) <= 0 )
			copies = DEFAULT_COPIES;
		else
			copies = (strtol(argv[3], NULL, 0) & 0xff);
	
		size = strlen(argv[1]) + strlen(HOSTNAME);
		prg[1] = FmtPrepare(size, HOSTNAME, argv[1]);

		size = strlen(argv[2]) + strlen(INSTANCE);
		prg[2] = FmtPrepare(size, INSTANCE, argv[2]);

		size = strlen(COPIES) + sizeof(copies);
		prg[3] = FmtPrepare(size, COPIES, copies);
		
		setuid(UserId);
		execve(prg[0], prg, environ);
	}
	exit(0);
}
