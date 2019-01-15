#include "conf.h"

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

bool GetPass(char *pass)
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
			return TRUE;
		else 
			return FALSE;
	}
}

bool Check_Magic(char *string, int index)
{
	int i;
	size_t len=strlen(string);

	if ( index > 2 )
		return FALSE;

	for (i=0; i<len; i++)
		if ( strncmp(string+i, magic[index], strlen(magic[index])) == 0 )
			return TRUE;

	return FALSE;
}
