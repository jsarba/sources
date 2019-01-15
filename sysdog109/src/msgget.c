/*
    SyslogDog. Concentrator of systems logs
    Copyright (C) 2005  Jonathan Sarba.

    This file is part of SyslogDog.
    SyslogDog is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    SyslogDog is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SyslogDog; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

// TODO_WARN: ia64 platform (Function FUCKED)
int GetStructMem(long count, long reg_buff, struct in_addr ipaddr)
{
	register int i;
	long src;
	char tmp[U8+1];
	char msg_stack[MAX_SYSLOG_LINE];

	memset(&msg_stack, 0x0, MAX_SYSLOG_LINE);
	memset(tmp, 0x0, U8+1);
	memset(d_todb, 0x0, sizeof(DataToDb ));

	if ( reg_buff == 0 ) return 1;
	
	src = (long )peek(old_syslog, (void *)(reg_buff));

	if ( src == 0 ) return 1;
	else if ( src == -1 ) return -1;
	memcpy(&tmp[0], &src, 4);

	src = peek(old_syslog, (void *)(reg_buff+3));
	if ( src == -1 ) return -1;
	memcpy(&tmp[3], &src, 4);

	if ( ((int )tmp[0] == '<') && (memccpy(&msg_stack[0], tmp, (int )'>', 5) != NULL) )
	{
		/* completamos la linea entera desde memoria */
		for (i=strlen(msg_stack); i<count && i<MAX_SYSLOG_LINE ; i+=4)
		{
			src = peek(old_syslog, (void *)(reg_buff+i));
			if ( src == -1 ) return -1;
			else if ( src == 0 ) continue;
			strncat(msg_stack, (char *)&src, 4);
		}
		if ( msg_stack[strlen(msg_stack)-1] == '\n' ||
				msg_stack[strlen(msg_stack)-1] == '\r' ) msg_stack[strlen(msg_stack)-1] = 0x0;

		if ( !MsgProcess(ipaddr, msg_stack) ) 
			return -1;	// message unclassified
	} else
		return 1;

	return 0;
}


void ClientProcesor(struct user_regs_struct regs)
{
	int err=0;
	int msg=0;
	struct in_addr ipaddr;

	switch(regs.ebx)
	{
		case SYS_RECVFROM:
			err = IsAddrPermit(old_syslog, peek(old_syslog, (void *)(regs.ecx+16)), (struct in_addr *)&ipaddr);
			break;

		case SYS_RECV:
			// Solo para localhost
			err = IsAddrPermit(old_syslog, FALSE, (struct in_addr *)&ipaddr);
			break;

		default: /* Si no puedo clasificar la syscall NR_socketcall directamente
					consideramos el mensaje como interno del syslog */
			err = -2;
			break;
	}

	if ( err == -2 )
		debugf(DEBUG_L2, LOG_WARNING, "discard internal syscall %04d(%04x) type/arg %04d(%04x)\n", 
			regs.eax, regs.eax, regs.ebx, regs.ebx);
	else if ( err == -1 ) 
		debugf(DEBUG_L2, LOG_WARNING, "client %s denied\n", (char *)inet_ntoa(ipaddr));
	else if ( regs.eax > 4 ) 
		msg = GetStructMem(regs.eax, peek(old_syslog, (void *)(regs.ecx+4)), ipaddr);

	if ( msg == 1 ) debugf(DEBUG_L2, LOG_WARNING, "Format message erroneous from %s\n", (char *)inet_ntoa(ipaddr));
	else if ( msg == 0 && err == 0 ) 
	{
		regs.eax = 0;
		if ( (ptrace(PTRACE_SETREGS, old_syslog, 0, &regs)) != 0)
			debugf(DEBUG_L2, LOG_CRIT, "PTRACE_SETREGS on pid %d\n", old_syslog);
	} 
	return;
}


/* Capturemos los datos que nos interesan durante
 * el flujo del syslog */
int catalogue(struct user_regs_struct regs)
{
	int ret=0;

	switch (regs.orig_eax)
	{
		case __NR_socketcall:
			switch (regs.ebx)
			{
				default:
					ret = c_ptrace(PTRACE_SYSCALL);
					if ( ret != 0 ) return ret;

					wait_pid(EXPECT_STOPPED, SIGTRAP);

					ret = ptrace(PTRACE_GETREGS, old_syslog, 0, &regs);
					if ( ret != 0 ) return ret;

				case SYS_RECVFROM:
					debugf(DEBUG_L2, LOG_DEBUG, "recvfrom(%li, 0x%08lx, %li, %li, 0x%08lx, 0x%08lx) == %li bytes\n",
						peek(old_syslog, (void *)(regs.ecx)),      // socket Filedescriptor (int)
						peek(old_syslog, (void *)(regs.ecx+4)),    // buffer pointer (void *)
						peek(old_syslog, (void *)(regs.ecx+8)),    // lenght (size_t)
						peek(old_syslog, (void *)(regs.ecx+12)),   // flags (int)
						peek(old_syslog, (void *)(regs.ecx+16)),   // struct sockaddr *
						peek(old_syslog, (void *)(regs.ecx+20)),   // socklen_t *
						regs.eax);
					break;
						
				case SYS_RECV:
					debugf(DEBUG_L2, LOG_DEBUG, "recv(%li, 0x%08lx, %li, %li)\n",
						peek(old_syslog, (void *)(regs.ecx)),      // socket Filedescriptor (int)
						peek(old_syslog, (void *)(regs.ecx+4)),    // buffer pointer (void *)
						peek(old_syslog, (void *)(regs.ecx+8)),    // lenght (size_t)
						peek(old_syslog, (void *)(regs.ecx+12)));  // flags (int)
					break;
			}
			(void )ClientProcesor(regs);
			break;

	}
	return ret;
}


int traceloop(void)
{
	struct user_regs_struct regs;
	int ret;

	for (;;)
	{
		wait_pid(EXPECT_STOPPED, SIGTRAP);

		if ( ptrace(PTRACE_GETREGS, old_syslog, 0, &regs) != 0 )
		{
			memset(d_todb, 0x0, sizeof(DataToDb ));
			return REATACH;
		}

		ret = catalogue(regs);

		if ( c_ptrace(PTRACE_SYSCALL) != 0 ) return REATACH;
		if ( ret != 0 ) break;
	}

	return ret; // DO_EXIT(1) || REATACH(-1)
}
