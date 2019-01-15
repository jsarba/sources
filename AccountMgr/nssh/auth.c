#include "auth.h"

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
