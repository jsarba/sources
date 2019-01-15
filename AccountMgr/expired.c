/*
 * Jonathan Sarba
 *
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

extern int passwdexpired(char *, char **);

int main(int argc, char **argv)
{
	uid_t UserId;
	int expire_code;
	struct passwd *user;
	char *expire_msg;

	if ( argc < 1 ) 
		return printf("use: %s <username>\n", argv[0]);

	if ( argv[1] )
	{
		if ( (user = getpwnam(argv[1])) == NULL )
			return -1;
	}
	else	
	{
		UserId = getuid();
		if ( (user = getpwuid(UserId)) == NULL )
			return -1;
	}

	expire_code = passwdexpired((char *)user->pw_name, &expire_msg);

	switch (expire_code)
	{
		case -1:
			printf("%s: %s", user->pw_name, expire_msg);
			printf("Internal error or database corruption.\n");
			break;
		case 2:
		case 1:
			if ( expire_msg && *expire_msg )
			{
				char *ptr;
				for ( ptr=expire_msg ; *ptr; ptr++ )
					if ( *ptr == '\n' ) *ptr = ' ';

				*--ptr = '\0';
			}
			printf("%s: %s\n" , user->pw_name, expire_msg);
			break;
	}	
	exit(0);
}

