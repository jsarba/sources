/*
 * Jonathan Sarba
 * 25 / 11 / 2003
 *
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#define MAXLINE	4096
#define MAXFILE	255

int main(int argc, char **argv)
{
	FILE	*fd;
	char	*line;
	unsigned long cl, nl, x=0;
	
	if ( argc < 4 )
	{
		printf("use: %s [file] [line] [# lines]\n", argv[0]);
		exit(0);
	}

	nl = strtoul(argv[2], NULL, 0);
	cl = nl + strtoul(argv[3], NULL, 0);

	if ( strlen(argv[1]) > MAXFILE || nl <= 0 || cl < 0 )
		exit(-1);

	if ( (fd=fopen(argv[1], "r")) == NULL ) 
		exit(-2);

	if ( (line=(char *)calloc(MAXLINE+1, sizeof(char))) == NULL )
		exit(-3);

	while ( fgets(line, MAXLINE, fd) )
	{
		x++;
		if ( x >= nl && x < cl )
			printf("%s", line);
	}
	
	fclose(fd);
	exit(0);
}
