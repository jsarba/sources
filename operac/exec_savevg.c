/*
 * Jonathan Sarba
 * 27 / 11 / 2003
 * 
 * Execution of /usr/bin/savevg for AIX platforms
 * set perm to 4750
 *
 * Uso: ./exec_savevg /dev/rmt1 datavg
*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define	FMT	"-f %s -e -i -X -p %s"
#define	PRG	"/usr/bin/savevg"
#define MAX	512

int main(int argc, char **argv)
{
	int 	size;
	uid_t	UserId;
	char	*prg[2]={NULL};
	
	UserId = geteuid();

	if ( argc < 3 )
	{
		printf("use: %s [tape] [vg]\n", argv[0]);
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

		size = strlen(argv[1]) + strlen(argv[2]) + strlen(FMT);

		if ( size > MAX )
			exit(-1);

		if ( (prg[1]=(char *)calloc(size + 1, sizeof(char))) == NULL ) 
			exit(-1);
		
		if ( sprintf(prg[1], FMT, argv[1], argv[2]) < 0 )
			exit(-1);
		
		setuid(UserId);
		execve(prg[0], prg, NULL);
	}
	exit(0);
}
