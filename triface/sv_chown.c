/*
 * Jonathan Sarba
 * 12 Julio de 2005
 *
*/
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pwd.h>
#include <errno.h>

#define MAX 512
#define MAXUSERS 1

#define ADMIN_FILE "/etc/triface.own"

extern char **environ;
static char *list[MAXUSERS];

char *GetFileName()
{
    char *work;
    char *auth;

    if ( (auth=(char *)calloc(MAX, sizeof(char))) == NULL )
        exit(-2);

    work = getenv("WORK_HOME");

    if ( !work || (strlen(work)+strlen(ADMIN_FILE)) > MAX )
        return NULL;

    memcpy(auth, work, strlen(work));
    memcpy(auth+strlen(auth), ADMIN_FILE, strlen(ADMIN_FILE));

    return auth;
}


int LoadList(const char *file, uid_t user)
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
		if ( filestat.st_uid != user )
		{
			printf("File %s must be set %ld owner\n", file, user);
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
        memcpy(line+=point, (char *)&tmp[0], strlen(tmp));

        point += strlen(tmp) + 1;
        list[c] = (char *)line;
        c++;
    }

    fclose(fd);
    return 0;
}



int getusrinfo(const char *owner, uid_t *usr)
{
	struct passwd *own_info;

	if ( (own_info = getpwnam(owner)) == NULL )
		return 0;
	else
		*usr = own_info->pw_uid;
	return 1;
}


int main(int argc, char **argv)
{
	int auth = 0, i;
	uid_t realuser, UserId;
	char *admin_file;

	if ( argc < 2 )
	{
		printf("use: %s [path] <own>\n", argv[0]);
		exit(0);
	}

	// setuid de root
	if ( geteuid() != 0 )
	{
		printf("File must be setuid for root user\n");
		exit(-1);
	} 

   	if ( (admin_file = GetFileName()) == NULL )
  	{
		printf("env WORK not defined\n");
		exit(-1);
   	}

   	if ( LoadList(admin_file, 0) )
   	{
       	printf("Can't open file %s\n", admin_file);
       	exit(-1);
   	}

	if ( !argv[2] || !getusrinfo(argv[2], &realuser) || realuser == 0 )
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
		printf("The userid %ld is not authorized to change owner file\n", realuser);
		exit(-2);	
	}

	if ( chown(argv[1], realuser, -1) == -1 )
		perror("chown");

	exit(0);
}
