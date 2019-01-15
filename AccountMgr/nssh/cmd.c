#include "cmd.h"

bool WritePassword(int mark, CHANNEL *channel)
{
	static int tms = 0;

	if (tms != mark)
	{
		channel_write(channel, pass2[(mark>1)?1:0], strlen(pass2[(mark>1)?1:0]));
		tms = mark;
	} else
		return FALSE;

	return TRUE;
}

bool Send_Command(SSH_SESSION *session, char *cmd)
{
	BUFFER *buf = buffer_new();
	CHANNEL *channel = NULL;
	struct timeval timeout;
    static int mark = 0, gic = 0;
	fd_set fds;
	int p = 0, ret = 0;
	
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

		do
		{
			FD_ZERO(&fds);
			FD_SET(0, &fds);
			timeout.tv_sec = 0; 
			timeout.tv_usec = 0;

			// [3] si hemos enviado la password entonces enviar MAGIC 
			if ( gic )
			{
                channel_write(channel, MAGIC, 1);
				gic = 0;
			} else
				ret = ssh_select(chans, ochans, 1, &fds, &timeout);

			if ( !gic && mark )
				if ( (gic = WritePassword(mark, channel)) )
					channel_write(channel, MAGIC, 1);

		} while ( ret == SSH_EINTR );
	
		/* nunca deberia de entrar aca  */
		if ( FD_ISSET(0, &fds) )
			if ( (gic = WritePassword(mark, channel)) )
                channel_write(channel, MAGIC, 1);

		if ( ochans[0] )
		{
			while ( channel_poll(ochans[0], 0) )
			{
				// stdout read 
				p = channel_read(ochans[0], buf, 0, 0);
				if ( p ) 
				{
					// [1] mark sumara si encuentra la palabra buscada
					if ( mark < 3 )
						mark = Check_Magic((char *)buffer_get(buf));

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
