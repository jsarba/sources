#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <stdio.h>
#include <pwd.h>

#define MAX 512
#define AUTH_FILE "/etc/auth_admin"
#define COMM_FILE "/etc/comm_admin"

extern char **environ;

char *GetFileName(char *obj)
{
	char *path;
	char *file;
	
	if ( (file=(char *)calloc(MAX, sizeof(char))) == NULL )
		exit(-2);

	path = getenv("WORK");

	if ( !path || (strlen(path)+strlen(obj)) > MAX )
		return NULL;

	memcpy(file, path, strlen(path));
	memcpy(file+strlen(file), obj, strlen(obj));

	return file;
}	

/* bool */
int CheckValidCommand(char *arg)
{
	FILE *fd;
	char *comm_file;
	char tmp[MAX];
	struct stat st_file;

	comm_file = GetFileName(COMM_FILE);

	if ( stat(comm_file, &st_file) == -1 )
		return 0;

	if ( (fd = fopen(comm_file, "r")) == NULL )
		return -0;

	while (fgets(tmp, MAX-1, fd))
	{
		tmp[strlen(tmp)-1] = '\0';
	        if ( strncmp(tmp, "#", 1) == 0 ) continue;
		if ( strncmp(tmp, arg, strlen(arg)) == 0 )
		{
			fclose(fd);
			return 1;
		}
	}

	fclose(fd);
	return 0;
}

uid_t GetUserAuthorized(uid_t user)
{
	FILE *fd;
	char *auth_file;
	char tmp[MAX];
	struct passwd *owner;
	struct stat st_file;

	auth_file = GetFileName(AUTH_FILE);

	if ( stat(auth_file, &st_file) == -1 )
		return -1;
		
	if ( (fd = fopen(auth_file, "r")) == NULL )
		return -1;

	while ( fgets(tmp, MAX-1, fd) )
	{
		tmp[strlen(tmp)-1] = '\0';

        	if ( strncmp(tmp, "#", 1) == 0 || strncmp(tmp, "!", 1) != 0 ) 
		{	
			bzero(tmp, MAX);
			continue;
		}

		if ( (owner = getpwnam(tmp)) == NULL )
			break;

		if ( owner->pw_uid == user ) 
		{
			fclose(fd);
			return owner->pw_uid;
		}
		bzero(tmp, MAX);
	}

	fclose(fd);
	return -1;
}


int main(int argc, char **argv) 
{
	int i;
	uid_t euid, uid;
	char *ptr[argc];

	euid = geteuid();
	if ( euid != 0 )
		return printf("File must be setuid for root\n");		

	uid = getuid();
	if ( GetUserAuthorized(uid) <= 0 )
		return printf("UID %ld is not authorized to execute this file\n", uid);
	
	if ( argc < 2 || !CheckValidCommand(argv[1]))
		return printf("Invalid command\n");

	for (i=1; i < argc; i++)
		ptr[i-1] = argv[i];

	ptr[i-1] = NULL;

	setuid(0);
	execve(ptr[0], ptr, environ);
	exit(0);	
}

