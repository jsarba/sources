/*
 * Jonathan Sarba
 * 20 Junio de 2005
 * Version 1.4 
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <strings.h>
#include <memory.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pwd.h>

#define MAX	256
extern char **environ;

#define TSM_RSTART 1
#define LST_RSTART 2

char *prg[] = { "/usr/local/bin/tsm.sh", "/bin/lsnrctl", NULL };

void use(void)
{
		printf("Case options [#]:
\t[1] restart tsm service
\t[2] restart listener service(Oracle) 
\t\t<arg>: [dba_owner]
");
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

        
char *path_complete(char *var, char *command)
{
	char *path;
	char *ret;

	if ( (ret=(char *)calloc(MAX+strlen(command), sizeof(char))) == NULL )
		exit(-2);

	path = getenv(var);

	if ( !path || (strlen(path)+strlen(var)) > MAX )
		return NULL;

	memcpy(ret, path, strlen(path));
	memcpy(ret+strlen(ret), command, strlen(command));

	return ret;
}


int exec_command(uid_t id, char *run, int n_args, ...)
{
	int ret = 1;
	int dad;
	va_list ap;
	struct stat tmp;
	char *ptr[n_args+1];
	
	if ( stat(run, &tmp) == -1 ) return 0;
	ptr[0]=run;

	va_start(ap, n_args);
	for( ret=0 ; ret < n_args; ret++ )
		ptr[ret+1] = va_arg(ap, char *);
	va_end(ap);

	ptr[ret+1] = NULL;

	if ( (dad = fork()) == -1 ) exit(-1);
	setuid(id);

	if ( dad == 0 )
		execve(ptr[0], ptr, environ);
  
	while ( waitpid(0, NULL, 0) != dad );

	return ret;
}


int main(int argc, char **argv)
{
	int option;
	int sel;
	uid_t UserId, OraId;
	char *ptr;

	if ( argc < 2 )
	{
		printf("use: %s [#] <args ...>\n", argv[0]);
		(void )use();
		exit(0);
	}

	if ( argv[1] ) option = strtol(argv[1], NULL, 0);
	else exit(-1);

	sel = option-1;
	UserId = geteuid();
	
	if ( UserId != 0 )
	{
		printf("File must be setuid for root owner\n");
		exit(-1);       
	}
        
	switch(option)
	{
		case TSM_RSTART:
			if ( !exec_command(UserId, prg[sel], 1, "stop") ||
				!exec_command(UserId, prg[sel], 1, "start") )
				printf("execution %s error or command not found\n", prg[sel]);
			break;

		case LST_RSTART:
			if ( !argv[2] || !getusrinfo(argv[2], &OraId) ) 
			{
				printf("User \"%s\" not found or not defined\n", argv[2]);
				exit(-1);
			}

			ptr = path_complete("ORACLE_HOME", prg[sel]);

			if ( !ptr )
			{
				printf("env ORACLE_HOME not defined\n");
				exit(-1);
			}

			if ( !exec_command(OraId, ptr, 1, "stop") ||
				!exec_command(OraId, ptr, 1, "start") )
				printf("execution %s error or command not found\n", prg[sel]);
			
			break;

		default:
			printf("Invalid option (%d)\n\n", option);
			(void )use();
			exit(-1);
			break;
	}
			
	exit(0);
			
}

