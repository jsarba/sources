/*
 * Jonathan Sarba
 *
*/
#include <stdio.h>
#include <userpw.h>
#include <usersec.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
	char attr[20];
	char *expire_code;
	long expire;

	if ( argc < 1 ) 
		return printf("use: %s [username]\n", argv[0]);

	if ( !argv[1] )
		return -1;

	if ( setuserdb(S_READ) < 0 )
		return -1;

	strncpy(attr, S_EXPIRATION, strlen(S_EXPIRATION));

	getconfattr(argv[1], S_EXPIRATION, expire_code, SEC_CHAR);

	printf("%s %s\n", argv[1], expire_code);

	if ( enduserdb() != 0 ) return -1;

	exit(0);
}

