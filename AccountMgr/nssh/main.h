#ifndef _MAIN_H_
#define _MAIN_H_

#include "common.h"
#include "hssh.h"

extern void usage(char *);
extern char *GetLastArg(int, char **);
extern int handshake(SSH_SESSION *);
extern int authentication(SSH_SESSION *);
extern bool Send_Command(SSH_SESSION *, char *);

#endif
