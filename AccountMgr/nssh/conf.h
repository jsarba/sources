#ifndef _CONF_H_
#define _CONF_H_

#include "common.h"

#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define _PATH_TTY       "/dev/tty"

static char *magic[] = { "assword:", "New passw", "ssword again:", NULL };

// bool GetPass(char *);
// int echoing(void);

int Check_Magic(char *);

#endif
