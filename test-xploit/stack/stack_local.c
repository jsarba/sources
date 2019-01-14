#include<stdio.h>
#include<string.h>
int main(int argc,char *argv[])
{

    char buf[256];
    if (argc != 2){ return -1; }//Cuantos Argumentos Tenemos
    strcpy(buf,argv[1]); 	// Copio argv[1] sin chequear len <-  
    return 0x0;
}

