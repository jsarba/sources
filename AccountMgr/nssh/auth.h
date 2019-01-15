#ifndef _AUTH_H_
#define _AUTH_H_

#include "common.h"
#include "hssh.h"

extern bool GetPass(char *);

int handshake(SSH_SESSION *);
int authentication(SSH_SESSION *);

#endif
