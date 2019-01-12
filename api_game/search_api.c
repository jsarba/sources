/*
 * Matias Sedalo <s0t4ipv6@shellcode.com.ar>
 * http://www.shellcode.com.ar
 * (c) 1999 - 2004 ShellCode Research.
 *
 * Compile:
 * gcc -Wall -o example-libs.exe example-libs.c searchDLL.obj searchFunction.obj 
 * 
 * Use:
 * >example-libs LoadLibraryA kernel32
 * [*] Search 'LoadLibraryA' on kernel32(0x77e80000)
 * [*] Found at 0x77E89F64
 * 
 */
#include <stdio.h>
#include <stdlib.h>

#define	u_long unsigned long

// 
extern u_long FindFunctionOnDllAddr(u_long, char *, long);
extern u_long FindLoadedDllAddress(char *);

int main(int argc, char *argv[])
{
	u_long dll_base;
	u_long addr_func;
	long lenght;
	char *Win32Api;
	char *DLLName;

	if ( argc < 3 ) 
	{
		printf("\nAPISearch+(AsmLib)x4.2 by s0t4ipv6@shellcode.com.ar\n");
		printf("use: %s [Win32Api] [ntdll.dll|kernel32.dll|msvcrt.dll]\n", argv[0]);
		exit(-1);
	}

	Win32Api = argv[1];
	DLLName = argv[2];

	dll_base = FindLoadedDllAddress(DLLName);

	if ( !dll_base )
	{
			printf("[!] '%s' not found on memory\n", DLLName);
			exit(-1);
	}

	printf("\n[*] Search '%s' on %s(0x%lx)\n", Win32Api, DLLName, dll_base);

	lenght = strlen(Win32Api);
	addr_func = FindFunctionOnDllAddr(dll_base, Win32Api, lenght);

	if ( !addr_func )
	{
		printf("[!]'%s' not found on KernelBase(0x%lx)\n", Win32Api, dll_base);
		exit(-1);
	} else {
		printf("[*] Found at 0x%lX\n", addr_func);
	}

	exit(0);
}
