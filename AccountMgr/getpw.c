#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *run0="\x4f\x67\x5a\x30\x4f\x44\x5a\xf";

int main(int argc, char **argv)
{
        int i;
        int r;
        int sid;
        char *pass;

        if ( argc < 2 )
                exit(-1);

        if ( argv[2] )
        {
            sid = strtoul(argv[2], NULL, 0);
            pass = argv[1];
        } else {
            sid = strtoul(argv[1], NULL, 0);
            pass = run0;
        }

        if ( sid == 0 ) exit(-1);

        for (i=0; i<strlen(pass); i+=2)
        {
                r = (int)pass[i] ^ sid;
                printf("%c", r);;
        }

        printf("\n");
        exit(0);
}

