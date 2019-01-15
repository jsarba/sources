#ifndef _CMD_H_
#define _CMD_H_

#include "common.h"
#include "hssh.h"

#include <sys/time.h>
#include <sys/types.h>

#define MAGIC   "\n"

extern int echoing(void);
extern bool Check_Magic(char *, int);

bool Send_Command(SSH_SESSION *, char *);

static char *pass[] = { "R3c3nt S0ngs", "niseteocurra", NULL };

#endif 
