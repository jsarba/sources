#include "main.h"

int main(int argc, char **argv)
{
	FILE *fd;
	SSH_OPTIONS *options;
	SSH_SESSION *sessions;
	const char *hostname;
	const char *filename;
	char cmd[MAX_COM];

	options = ssh_getopt(&argc, argv);

	if ( !options )
	{
		printf("[!] %s\n", ssh_get_error(NULL));
		usage(argv[0]);
	}

	if ( (hostname = GetLastArg(argc, argv)) == NULL )
		usage(argv[0]);

	if ( (filename = GetLastArg(argc, argv)) == NULL )
		usage(argv[0]);

	if ( (fd = fopen(filename, "r")) == NULL )
	{
		perror("[!]");
		return -1;
	}

	options_set_host(options, hostname);

	if ( (pass2[0] = GetLastArg(argc, argv)) == NULL )
		usage(argv[0]);

	if ( (pass2[1] = GetLastArg(argc, argv)) == NULL )
		usage(argv[0]);

	if ( (sessions = ssh_connect(options)) == NULL )
	{
		printf("[!] %s\n", ssh_get_error(NULL));
		return (fd) ? fclose(fd) : 0 ;
	}

	if ( !handshake(sessions) )
	{
		printf("[!] %s\n", ssh_get_error(NULL));
		(void )ssh_disconnect(sessions);
		if ( fd ) fclose(fd);
		exit(-1);
	}
		
	if ( !authentication(sessions) )
	{
		(void )ssh_disconnect(sessions);
		if ( fd ) fclose(fd);
		exit(-1);
	}
	
	while ( fgets(cmd, MAX_COM-1, fd ) ) 
	{
		cmd[strlen(cmd)-1] = '\0';
		if ( strlen(cmd) < 2 || strncmp(cmd, "#", 1) == 0 ) continue;
		if ( strncmp(cmd, "<", 1) == 0 )
		{
			printf("%s\n", cmd);
			continue;
		}
	
		if ( !Send_Command(sessions, cmd) )
			printf("[!] %s\n", ssh_get_error(NULL));
	}

	printf("[*] Disconnected\n");
	(void )ssh_disconnect(sessions);
	if ( fd ) fclose(fd);
	exit(0);
}
