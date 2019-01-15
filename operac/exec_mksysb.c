/*
 * Jonathan Sarba
 * 25 / 11 / 2003
 * 
 * Execution of /usr/bin/mksysb for AIX platforms
 * set perm to 4750
 * 
 * Uso: ./exec_mksysb /dev/rmt1
 *
*/
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory.h>

#define TAPE            "/dev/rmt0"
#define ARGUMENT        "-e -i -X -p"
#define PROGRAM         "/usr/bin/mksysb"

int main(int argc, char **argv)
{
        uid_t   UserId;
        char    *prg[]={PROGRAM, ARGUMENT, TAPE, NULL};

        UserId = geteuid();

        if ( argv[1] && strlen(argv[1]) < 10 )
                memcpy(&prg[2], &argv[1], 9);

        if ( UserId != 0 )
        {
                printf("File must be setuid for root owner\n");
                exit(-1);
        }
        else
        {
                setuid(UserId);
                execve(prg[0], prg, NULL);
        }

        exit(0);
}
