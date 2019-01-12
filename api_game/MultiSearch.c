/*
 * Matias Sedalo <s0t4ipv6@shellcode.com.ar>
 * http://www.shellcode.com.ar
 * (c) 1999 - 2004 ShellCode Research.
 *
 * Compile DLL lib:
 * nasmw -fwin32 -o SearchDLL.obj SearchDLL.s
 *
 * gcc -Wall -o msearch.exe MultiSearch.c searchDLL.obj 
 * 
 * Use:
 * >msearch sprintf ntdll
 * >msearch Load kernel32
 * 
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winnt.h>

#define	RVA(BASE, adr)	(BASE+adr)
extern DWORD FindLoadedDllAddress(char *);

int main(int argc, char **argv)
{
	int x=0, idx;
	char *c;
	char *Function;
	char *DLL;
	DWORD BASE;		// old #define
	UCHAR **sym;
	UINT *addr;
	USHORT *ord;
	IMAGE_DOS_HEADER *idh;
	IMAGE_NT_HEADERS *nt;
	IMAGE_EXPORT_DIRECTORY	*ied;

	if ( argc < 3 )
	{
		printf("MultiSearch x2.2 by s0t4ipv6@shellcode.com.ar\n");
		printf("use: %s [pattern of function] [ntdll.dll|kernel32.dll|msvcrt.dll]\n", argv[0]);
		exit(0);
	}

	Function = argv[1];
	DLL = argv[2];
	BASE = FindLoadedDllAddress(DLL);

	if ( !BASE )
	{
		printf("[!] '%s' not found on memory\n", DLL);
		exit(-1);
	} else {
		printf("[*] Base Address of '%s' \t0x%lX\n\n", DLL, BASE);
	}

	idh = (IMAGE_DOS_HEADER *)BASE;
	if ( IMAGE_DOS_SIGNATURE != idh->e_magic ) exit(0);
	nt=(IMAGE_NT_HEADERS *)(RVA(BASE, (DWORD )idh->e_lfanew));
	if ( nt->Signature != IMAGE_NT_SIGNATURE ) exit(0);
#ifdef _DEBUG
	printf("[+] IMAGE_DOS_SIGNATURE \t%02X .... ok\n\n", idh->e_magic);
	printf("[*] Offset %08lx ->\n", idh->e_lfanew);
	printf("\tIMAGE_NT_HEADERS \t0x%lX\n", (DWORD )nt);
	printf("\tIMAGE_NT_SIGNATURE \t%02X .... ok\n", (WORD )nt->Signature);
#endif
	ied=(IMAGE_EXPORT_DIRECTORY *)(RVA(BASE, nt->OptionalHeader.DataDirectory[0].VirtualAddress));
	addr = (UINT *)(RVA(BASE, (DWORD )ied->AddressOfFunctions));
	(UCHAR *)sym=(UCHAR *)(RVA(BASE, (DWORD )ied->AddressOfNames));
	ord=(USHORT *)(RVA(BASE, (DWORD )ied->AddressOfNameOrdinals));
#ifdef _DEBUG
	printf("\n\tIMAGE_EXPORT_DIRECTORY \t0x%lX\n", (DWORD )ied);
	printf("\tNumber of Functions \t%ld\n", ied->NumberOfFunctions);
	printf("[*] Offset Functions 0x%lX ->\n", (DWORD )ied->AddressOfFunctions);
	printf("\tAddressOfFunctions \t0x%lX\n", (DWORD )addr);
	printf("[*] Offset Names 0x%lX ->\n", (DWORD )ied->AddressOfNames);
	printf("\tAddressOfNames \t\t0x%p\n", sym);
	printf("[*] Offset ONames 0x%lX ->\n", (DWORD )ied->AddressOfNameOrdinals);
	printf("\tAddressOfNameOrdinals \t0x%X\n\n", (UINT )ord);
#endif

	for (idx=0; idx < ied->NumberOfFunctions; idx++)
	{
		c = (char *)(RVA(BASE, sym[idx]));

		printf("\r[*] Search %s at 0x%lX", Function, (DWORD )RVA(BASE, addr[ord[idx]]));
		if (strncmp(c, Function, strlen(Function))==0)
		{
			x++;
			printf("\r[*] Found %s at 0x%lX .... ok\n", c, (DWORD )RVA(BASE, addr[ord[idx]]));
			_sleep(1);
		}
	}

	printf("\r\t\t\t\t\t");
	if (!x) printf("\r[!] Not found\t\t\t\t\n");
	exit(-1);
}

