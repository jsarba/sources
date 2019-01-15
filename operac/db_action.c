/*
 * Jonathan Sarba
 * 18 de octubre de 2004
 * 
 * El usuario debera tener seteadas las variables de entorno.
 * ORACLE_HOME; ORACLE_SID
 *
*/
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pwd.h>

extern char **environ;

#define MAX 512
#define MAXUSERS 50

#define CONNECT "connect internal\n"
#define STARTUP CONNECT"startup\nexit\n"
#define SHUTDOWN CONNECT"shutdown immediate\nexit\n"
#define SVRMGRL "/bin/svrmgrl"
#define DBAUTH_FILE "/etc/db_auth"

static char *list[MAXUSERS]; 

char *GetFileName()
{
    char *work;
    char *db_auth;

    if ( (db_auth=(char *)calloc(MAX, sizeof(char))) == NULL )
        exit(-2);

    work = getenv("WORK");

    if ( !work || (strlen(work)+strlen(DBAUTH_FILE)) > MAX )
        return NULL;

    memcpy(db_auth, work, strlen(work));
    memcpy(db_auth+strlen(db_auth), DBAUTH_FILE, strlen(DBAUTH_FILE));

    return db_auth;
}


int LoadList(const char *file)
{
    FILE *fd;
    char tmp[MAX];
    char *line;
    int point = 0, c=0;
	struct stat filestat;

	if ( stat(file, &filestat) == -1 )
		return -1;
	else
	{
		if ( filestat.st_uid != 0 )
		{
			printf("File %s must be set root owner\n", file);
			exit(-1);
		}
	}

    if ( (fd = fopen(file, "r")) == NULL )
        return -1;

    if ( (line=(char *)calloc(4096, sizeof(char))) == NULL )
        exit(-2);

    (void )fseek(fd, 0L, SEEK_SET);

    while( fgets(tmp, MAX-1, fd) )
    {

        if ( c >= MAXUSERS || point >= 4096 )
        {
            list[c-1] = NULL;
            break;
        }

        if ( strncmp(tmp, "#", 1) == 0 ) continue;

        tmp[strlen(tmp)-1] = '\0';
        memcpy(line+=point, tmp, strlen(tmp));

        point += strlen(tmp) + 1;
        list[c] = (char *)line;
        c++;
    }

    fclose(fd);
    return 0;
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


int getusrinfo(const char *db_owner, uid_t *usr)
{
	struct passwd *dbainfo;

	if ( (dbainfo = getpwnam(db_owner)) == NULL )
		return 0;
	else
		*usr = dbainfo->pw_uid;
	return 1;
}


int main(int argc, char **argv)
{
	int fd[2], auth=0;
	int i, stop=0;
	uid_t realuser, UserId, OraId;
	pid_t pid;
	char *dbauth_file;
	char *prg[2] = { NULL };

	if ( argc < 4 )
	{
		printf("use: %s [database] [dba_owner] [stop|start]\n", argv[0]);
		exit(0);
	}
	if ( strncmp(argv[3], "stop", 4) == 0 ) stop = 1;

	if ( geteuid() != 0 )
	{
		printf("File must be setuid for root owner\n");
		exit(-1);
	} 

	if ( !getusrinfo(argv[2], &OraId) ) 
	{
		printf("User \"%s\" not found\n", argv[2]);
		exit(-1);
	}

	if ( stop == 1 )
	{
    	if ( (dbauth_file = GetFileName()) == NULL )
   		{
			printf("env WORK not defined\n");
        	exit(-1);
    	}

    	if ( LoadList(dbauth_file) )
    	{
        	printf("Can't open file %s\n", dbauth_file);
        	exit(-1);
    	}

		realuser = getuid();

		for(i=0 ; list[i] != NULL ; i++)
		{
			if ( getusrinfo(list[i], &UserId) && (UserId == realuser) )
			{
				auth = 1;
				break;
			}
		}

		if ( auth == 0 )
		{
			printf("The userid %ld is not authorized to stop the data base\n", realuser);
			exit(-2);	
		}
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

	setuid(OraId);

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
		prg[0] = svrmgrl();
		execve(prg[0], prg, environ);
	} 

	exit(0);
}
