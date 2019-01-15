#ifndef _ARG_H_
#define _ARG_H_

#include "common.h"

#define AUTHOR          "Jonathan Sarba<jsarba@petrobrasenergia.com>"
#define PROGRAM         "fssh"
#define VERSION         "v0.2"

extern char *optarg;
extern int optind, optopt;

void usage(char *);
char *GetLastArg(int, char **);

#endif

