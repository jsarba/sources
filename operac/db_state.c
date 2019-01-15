/*
 * Jonathan Sarba
 * 13 de octubre de 2004
 * 
 * El usuario debera tener seteas las sigueintes variables de entorno.
 * ORACLE_HOME ; ORACLE_SID
 *
*/
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <pwd.h>

/* fucking lies */
#define MAX 512
#define PID 0x4e
int iiv = 0;
/* hardcore xxx */
static char *run1="\x3e\x19\x3c\x63\x27\x2d\x23\x77\x2f\x40\x38\xc\x2b\x54\x3c\x1f\x2f\x68"; 
static char *run0="\x2d\x7\x21\x50\x20\x1b\x3d\x63\x3b\x2f\x22\x77\x3a\x42\x2f\xc";
extern char **environ;

#define QUERY "select 'dbuped' from dual;\n"
#define SQLPLUS "/bin/sqlplus"

inline char getuv(const char *line)
{
	char ret;

	if ( line[iiv+1] == 0 ) return NULL;
	ret = (char )(line[iiv] ^ PID);
	iiv += 2;

	return ret;
}

char *sqlplus()
{
	char *path;
	char *ret;

	if ( (ret=(char *)calloc(MAX, sizeof(char))) == NULL )
		exit(-2);

	path = getenv("ORACLE_HOME");

	if ( !path || (strlen(path)+strlen(SQLPLUS)) > MAX )
		exit(-2);
	
	memcpy(ret, path, strlen(path));
	memcpy(ret+strlen(ret), SQLPLUS, strlen(SQLPLUS));

	return ret;
}

int checksid(char *db)
{
	char *sid;

	sid = getenv("ORACLE_SID");

	if ( !sid || strncmp(sid, db, strlen(db)) != 0 ) return 0;
	else return 1;
}

int main(int argc, char **argv)
{
	int i, fd[2];
	pid_t pid;
	char *ptr;
	char *prg[2] = { NULL };

	if ( argc < 2 )
	{
		printf("use: %s [database]\n", argv[0]);
		exit(0);
	}
	
	if ( (ptr=(char *)calloc(MAX, sizeof(char))) == NULL )
		exit(-1);

	if ( pipe(fd) == -1 )
	{
		printf("Pipe error\n");
		exit(-2);
	}	

	if ( checksid(argv[1]) == 0 )
	{
		printf("env ORACLE_SID != database\n");
		exit(-2);
	}

	if ( (pid=fork()) == 0 )
	{
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		fprintf(stdout, "%s", QUERY);
	} else 
	{
		for(i=0 ; iiv < strlen(run0) && (ptr[i] = getuv(run0)); i++);
		iiv = 0 ; ptr[i++] = '/';
		for( ; iiv < strlen(run1) && (ptr[i] = getuv(run1)); i++);

		// No definimos el SID
		//ptr[i++] = '@';
		//memcpy(&ptr[i++], argv[1], strlen(argv[1]));

		prg[0] = sqlplus();
		prg[1] = ptr;

		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
		close(fd[1]);

		execve(prg[0], prg, environ);
		free(ptr);
	} 
	exit(0);
}
