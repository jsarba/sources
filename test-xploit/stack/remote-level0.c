/* stack overflow (strcpy) */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define WELCOME	"Demo - Stack Overflow\n"
#define DEFAULT	5074

void game_over(char *ovr)
{
	char overflow[1024];
	strcpy(overflow, ovr);
	printf("\nResponse:%s\n", overflow);
}

int main(int argc, char **argv)
{
	char via[2048];
	struct sockaddr_in saddr;
	struct sockaddr_in caddr;
	int soc, sr, client;
	socklen_t soclen;
	int port = DEFAULT;

	if ( argc > 1 ) port = atoi(argv[1]);

	if ( (soc=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1 )
		exit(-1);

	bzero((char *)&saddr, sizeof(saddr));

	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr= 0;
	saddr.sin_port=htons(port);

	if ( (sr = bind(soc, (struct sockaddr *)&saddr, sizeof(saddr))) != 0 )
		exit(-1);

	setpgrp();
	signal(SIGHUP, SIG_IGN);

	if ( (sr = listen(soc, 5)) != 0 )
		exit(-1);

	soclen = sizeof(caddr);
	if ( (client = accept(soc, (struct sockaddr *)&caddr, &soclen)) < 0 )
		exit(-1);
	write(client, WELCOME, sizeof(WELCOME));
	read(client, via, sizeof(via));

	dup2(client, 0);
	dup2(client, 1);
	dup2(client, 2);

	game_over(via);

	close(soc);
	exit(0);
}
