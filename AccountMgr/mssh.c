#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libssh/libssh.h>
#include <libssh/priv.h>
#include <errno.h>

#define AUTHOR          "Jonathan Sarba<jsarba@petrobrasenergia.com>"
#define PROGRAM         "fssh"
#define VERSION         "v0.1"

#define MAX_COM		2048
#define MAX_PASS	16
#define _PATH_TTY	"/dev/tty"

extern char *optarg;
extern int optind, optopt;

void usage(char *prgname)
{
	printf("%s %s (c) 2005 by %s\n\nUsage: %s [options] [host filelist]\n",
		PROGRAM, VERSION, AUTHOR, prgname);
  	printf("Options:\n"
    "  -l user : log in as user\n"
    "  -p port : connect to port\n");

  exit(-1);
}
int echoing(void)
{
	int infd;
	static int atom = 0;
	static struct termios oterm, nterm;
	
	if ( (infd = open(_PATH_TTY, O_RDWR)) == -1 )
		infd = (int )stdin;

	if ( atom == 1 )
	{
		(void )tcsetattr(infd, TCSAFLUSH, &oterm);
		return atom = 0;
	}
		
	if ( tcgetattr(infd, &oterm) == -1 )
		return atom;
	
	nterm = oterm;
	nterm.c_lflag &= ~(ECHO|ICANON);

	if ( tcsetattr(infd, TCSAFLUSH, &nterm) == -1 )
		return atom;

	return atom = 1;	
}

int GetPass(char *pass)
{
  	unsigned int p;
	int y;
	while(1)
	{
		y=0;
		memset(pass, 0x0, MAX_PASS);
		(void )echoing();	
		while ( (p = getc(stdin)) )
		{
			if (p == 0xa || p == 0xd) break;
			if (y < MAX_PASS && p != EOF) memcpy(pass+y, (char *)&p, 1);
			y++;
		}
		(void )echoing();
		if ( strlen(pass) > 0 && strlen(pass) < MAX_PASS ) 
			return 1;
		else 
			return 0;
	}
}

char *GetFileName(int argc, char **argv)
{
	int opt;
	while ( (opt=getopt(argc, argv, "")) != -1 )
	{
		switch(opt)
		{
			default:
				usage(argv[0]);
		}
	}
	if ( optind < argc )
		return argv[optind++];
	else 
		return NULL;
}

int handshake(SSH_SESSION *session)
{
	int state;
	char hash[MD5_DIGEST_LEN];
	
	state = ssh_is_server_known(session);

	switch(state)
	{
		case SSH_SERVER_KNOWN_OK:
			break;

		case SSH_SERVER_KNOWN_CHANGED:
			printf("[!] Host key for server changed: server's one is now:\n");
			(void )ssh_get_pubkey_hash(session, hash);
			(void )ssh_print_hexa("Public key hash", hash, MD5_DIGEST_LEN);
			return 0;

		case SSH_SERVER_FOUND_OTHER:
			printf("[!] The host key for this server was not found but an other type of key exists.\n"
					"    An attacker might change the default server key to confuse your client\n"
                	"    into thinking the key does not exist\n");
			return 0;

		case SSH_SERVER_NOT_KNOWN:
			printf("[!] The server is unknown. We'll trust this key\n");
			(void )ssh_get_pubkey_hash(session, hash);
			(void )ssh_print_hexa("Public key hash", hash, MD5_DIGEST_LEN);
			printf("[!] This new key will be written on disk for further usage.\n");	
			if ( ssh_write_knownhost(session) < 0 )
				return 0;
			break;

		case SSH_SERVER_ERROR:
			return 0;
	}

	return 1;
}



int authentication(SSH_SESSION *session)
{
	SSH_OPTIONS *opt = session->options;
	char *password;
	char *issue;
	int auth = 0;

	// try private/public keys first
	auth = ssh_userauth_autopubkey(session); 

	/* no quiero el banner
	if ( (issue = ssh_get_issue_banner(session)) )
		printf("%s", issue); 
	*/

	switch(auth)
	{
		case SSH_AUTH_DENIED:
			// printf("Error: no key matched\n");
		case SSH_AUTH_ERROR:
		case SSH_AUTH_PARTIAL:
			// printf("Error: some key matched but you still have to give an other mean of authentication\n");

 			if ( (password = (char *)calloc(MAX_PASS+1, sizeof(char))) == NULL)
				return 0;

			printf("%s@%s's password: ", opt->username, opt->host);
			if ( !GetPass(password) )
				printf("\n[!] Dont use authentication without strong password\n");

			if ( ssh_userauth_password(session, NULL, password) != SSH_AUTH_SUCCESS )
			{
				memset(password, 0x0, MAX_PASS);
				free(password);
				printf("\n[!] %s\n", ssh_get_error(session));
				return 0;
			}
			memset(password, 0x0, MAX_PASS);
			free(password);

		case SSH_AUTH_SUCCESS:
			printf("\n[*] Authentication success [%s on %s]\n", opt->username, opt->host);
			break;
	}

	return 1;
}

int Send_Command(SSH_SESSION *session, char *cmd)
{
	CHANNEL *channel = NULL;
	BUFFER *buf = buffer_new();
	struct timeval timeout;
	char ptr[10];
	fd_set fds;
	int p, ret;
	
	if ( (channel = channel_open_session(session)) == NULL )
		return 0;

	if ( channel_request_pty(channel) != 0 )
		return 0;

	if ( channel_request_exec(channel, cmd) != 0 )
		return 0;

		while(channel)
		{
			CHANNEL *chans[] = { channel, NULL } ;
			CHANNEL *ochans[2];

			(void )echoing();
			do
			{
				FD_ZERO(&fds);
				FD_SET(0, &fds);
				timeout.tv_sec = 2; 
				timeout.tv_usec = 0;
				ret = ssh_select(chans, ochans, 0+1, &fds, &timeout);
			} while ( ret == SSH_EINTR );
		
			if ( FD_ISSET(0, &fds) )
			{
				p = read(0, ptr, 10);	
				if ( p ) channel_write(channel, ptr, p);
			}
			(void )echoing();

			if ( ochans[0] )
			{
				while ( channel_poll(ochans[0], 0) )
				{
					p = channel_read(ochans[0], buf, 0, 0);
					if ( p ) write(1, buffer_get(buf), p);
				}
	
				while ( channel_poll(ochans[0], 1) )
				{
					p = channel_read(ochans[0], buf, 0, 1);
					if ( p ) write(2, buffer_get(buf), p);
				}
			}
			if ( !channel_is_open(channel) )
		 	{
		 		channel_free(channel);
		 		channel = NULL;
		 	}
		}
	buffer_free(buf);
	return 1;
}

int GetHostPort(FILE *fd, char *host, unsigned int *port, char *script)
{
	char tmp[MAX_COM] = {0};
	char Puerto[8] = {0};

	bzero(host, MAX_COM);
	bzero(script, MAX_COM);
	if ( fgets(tmp, MAX_COM-1, fd) )
	{
		if ( strncmp(tmp, "#", 1) == 0 ) 
			return GetHostPort(fd, host, port, script);
		
		memccpy(host, tmp, 0x3a, MAX_COM-1);
		host[strlen(host)-1] = '\0';

		memccpy(Puerto, tmp + strlen(host)+1, 0x3a, 8);
		if ( strlen(Puerto) >= 1 ) 
			Puerto[strlen(Puerto)-1] = '\0';

		*port = strtol(&Puerto[0], NULL, 0);

		memccpy(script, tmp + strlen(host)+strlen(Puerto)+2 , 0x3a, MAX_COM-1);
		if ( strlen(script) >= 1 )
			script[strlen(script)-1] = '\0';

		return 1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	FILE *fd, *flist;
	SSH_OPTIONS *options;
	SSH_SESSION *sessions = NULL;
	unsigned int port;
	char *host_list; 
	char cmd[MAX_COM];
	char hostname[MAX_COM], script[MAX_COM];

	options = ssh_getopt(&argc, argv);

	if ( !options )
	{
		printf("[!] %s\n", ssh_get_error(NULL));
		usage(argv[0]);
	}

	if ( (host_list = GetFileName(argc, argv)) == NULL )
		usage(argv[0]);

	if ( (flist = fopen(host_list, "r")) == NULL )
	{
		perror("[!]");
		return -1;
	}

	while ( GetHostPort(flist, hostname, &port, script) > 0 )
	{
		if ( (fd = fopen(script, "r")) == NULL )
		{
			printf("[!] No action for %s\n", hostname);
			continue;
		}
	
		options_set_host(options, hostname);
		printf("[*] Connecting to %s\n", hostname);
//		if ( port > 0 ) options_set_port(options, port);
	
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
	
		printf("[*] Disconnected from %s\n", hostname);

		(void )ssh_disconnect(sessions);
		if ( fd ) fclose(fd);
		options->fd = 0;
	}

	if ( flist ) fclose(flist);
	exit(0);
}
