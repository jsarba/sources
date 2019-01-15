/*
 * Jonathan Sarba
 * 13 de octubre de 2004
 * 
 * El usuario debera tener seteas las sigueintes variables de entorno.
 * ORACLE_HOME; ORACLE_SID
 *
*/
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <pwd.h>

#define MAX 512
extern char **environ;

#define CONNECT "connect internal\n"
#define STARTUP CONNECT"startup\nexit\n"
#define SHUTDOWN CONNECT"shutdown immediate\nexit\n"
#define SVRMGRL "/bin/svrmgrl"

gid_t getdbagrp(const char *db_owner)
{
	struct passwd *dbainfo;

	if ( (dbainfo = getpwnam(db_owner)) == NULL )
		return -1;
	else
		return dbainfo->pw_gid;
}

char *svrmgrl()
{
	char *path;
	char *ret;

	if ( (ret=(char *)calloc(MAX, sizeof(char))) == NULL )
		exit(-2);

	path = getenv("ORACLE_HOME");

	if ( !path || (strlen(path)+strlen(SVRMGRL)) > MAX )
		exit(-2);
	
	memcpy(ret, path, strlen(path));
	memcpy(ret+strlen(ret), SVRMGRL, strlen(SVRMGRL));

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
	int fd[2];
	int stop=0;
	gid_t GrpId, OraId;
	pid_t pid;
	char *prg[2] = { NULL };

	if ( argc < 4 )
	{
		printf("use: %s [database] [dba_owner] [stop|start]\n", argv[0]);
		exit(0);
	}
	if ( strncmp(argv[3], "stop", 4) == 0 ) stop = 1;

	GrpId = getegid();
	if ( (OraId = getdbagrp(argv[2])) == -1 )
	{
		printf("User \"%s\" not found\n", argv[2]);
		exit(-1);
	}

	if ( OraId != GrpId )
	{
		printf("File must be setgid for dba(%ld) group\n", OraId);
		exit(-1);
	}
	
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

		if ( stop == 1 )
			fprintf(stdout, "%s", SHUTDOWN);
		else
			fprintf(stdout, "%s", STARTUP);
	} else 
	{
		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
		close(fd[1]);

		setgid(OraId);
		prg[0] = svrmgrl();
		execve(prg[0], prg, environ);
	} 
	exit(0);
}
