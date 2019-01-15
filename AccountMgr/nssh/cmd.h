#ifndef _CMD_H_
#define _CMD_H_

#include "common.h"
#include "hssh.h"

#include <sys/time.h>
#include <sys/types.h>

#define MAGIC   "\n"

extern int echoing(void);
extern int Check_Magic(char *);

bool Send_Command(SSH_SESSION *, char *);
bool WritePassword(int, CHANNEL *);

#endif 
