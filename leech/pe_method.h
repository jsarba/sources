#ifndef _SANGUIJUELA_PE_METHOD_H
#define _SANGUIJUELA_PE_METHOD_H

void	AsignDinamicPoint			(HANDLE proc, DWORD addr);
DWORD	GetSectionFreeMemory		(HANDLE hProc, DWORD hMod, DWORD *f);
BOOL	CompleteInjApiStruct		(DWORD pos, DWORD addr, ExpList *InjApi);
DWORD	StaticInjectApiComplete	(DWORD pid, ExpList **InjApi);
DWORD	CompleteFreeMemList		(DWORD pid);

#endif // _SANGUIJUELA_PE_METHOD_H
