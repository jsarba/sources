#include "cmd.h"

bool Send_Command(SSH_SESSION *session, char *cmd)
{
	BUFFER *buf = buffer_new();
	char ptr[MAX_PASS] = { 0 };
	CHANNEL *channel = NULL;
	struct timeval timeout;
    int mark = 0, gic = 0;
	fd_set fds;
	int p, ret;
	
	if ( (channel = channel_open_session(session)) == NULL )
		return FALSE;

	if ( channel_request_pty(channel) != 0 )
		return FALSE;

	if ( channel_request_exec(channel, cmd) != 0 )
		return FALSE;

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

			// [3] si hemos enviado la password entonces enviar MAGIC 
			if ( mark && gic++ == 0)
				channel_write(channel, MAGIC, 1);

			ret = ssh_select(chans, ochans, 0+1, &fds, &timeout);
		} while ( ret == SSH_EINTR );
	
		if ( FD_ISSET(0, &fds) )
		{
			// [2] enviamos la password por el knal
			if ( mark )
				channel_write(channel, pass[(mark>1)?1:0], strlen(pass[(mark>1)?1:0]));
			else 
			{
				p = read(0, ptr, MAX_PASS);	
				if ( p ) channel_write(channel, ptr, p);
			}
		}
		(void )echoing();

		if ( ochans[0] )
		{
			while ( channel_poll(ochans[0], 0) )
			{
				// stdout read 
				p = channel_read(ochans[0], buf, 0, 0);
				if ( p ) 
				{
					// [1] mark sumara si encuentra la palabra buscada
					mark += Check_Magic((char *)buffer_get(buf), mark);
					write(1, buffer_get(buf), p);
				}
			}

			while ( channel_poll(ochans[0], 1) )
			{
				// stderr read
				p = channel_read(ochans[0], buf, 0, 1);
				if ( p ) 
					write(2, buffer_get(buf), p);
			}
		}
		if ( !channel_is_open(channel) )
	 	{
	 		channel_free(channel);
	 		channel = NULL;
	 	}
	}
	buffer_free(buf);
	return TRUE;
}
