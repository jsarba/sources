#include "common.h"
#include "misc.h"

FARPROC GetLibraryFunction(char *Library, char *Function)
{
	HMODULE lib32;

	if ( !Library || !Function ) return 0;

	lib32 = GetModuleHandle(Library);

	return GetProcAddress(lib32, Function);
}


DWORD GetTotalProcess()
{
	FARPROC kProcess32Next;
	PROCESSENTRY32 ProcEntry;
	HANDLE hEntry=0;
	DWORD ret=0;

	kProcess32Next = (FARPROC )GetLibraryFunction(KERNEL, "Process32Next");
	hEntry = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	while ( kProcess32Next(hEntry, &ProcEntry) != FALSE )
		ret++;

	return ret;
}


DWORD GetRemoteHandle(DWORD ProcID, char *shortLib)
{
	FARPROC kModule32Next; 
	MODULEENTRY32 ModEntry;
	HANDLE hEntry=0;
	DWORD ret=0;

	kModule32Next = (FARPROC )GetLibraryFunction(KERNEL, "Module32Next");
	hEntry = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcID);
	ModEntry.dwSize = sizeof(MODULEENTRY32);

	while( kModule32Next(hEntry, &ModEntry) != FALSE )
		if ( !shortLib )
		{
			ret = (DWORD )ModEntry.hModule;
			break;
		} else	
			if ( strncmp(ModEntry.szModule, shortLib, strlen(shortLib)) == 0 )
				ret = (DWORD )ModEntry.hModule;

	return ret;
}


DWORD CompleteGralProcess(char *ProcName, DWORD *Proc_list, int meth, BOOL patch)
{
	FARPROC kProcess32Next;
	PROCESSENTRY32 ProcEntry;
	HANDLE hEntry=0;
	DWORD ret=0;

	kProcess32Next = (FARPROC )GetLibraryFunction(KERNEL, "Process32Next");
	hEntry = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	ProcEntry.dwSize = sizeof(PROCESSENTRY32);

	while ( kProcess32Next(hEntry, &ProcEntry) != FALSE )
	{
		LowerCase(ProcEntry.szExeFile);
		if ( strncmp(ProcEntry.szExeFile, ProcName, strlen(ProcName)) == 0 )
			Proc_list[ret++] = (DWORD )ProcEntry.th32ProcessID;
	}
	
	return ret;
}


IMAGE_SECTION_HEADER *GetSectionHeader(IMAGE_NT_HEADERS *nt, BYTE section)
{
	DWORD offset = sizeof(struct _IMAGE_NT_HEADERS);

	if ( nt->Signature != IMAGE_NT_SIGNATURE ) return FALSE;

	if ( section > 0 )
		offset += sizeof(struct _IMAGE_SECTION_HEADER) * section;

	return (IMAGE_SECTION_HEADER *)(RVA((DWORD )nt, offset));
}


IMAGE_NT_HEADERS *GetNtHeaders(HANDLE hProc, DWORD ModuleHandle)
{
	BYTE buff[MIN_SIZE];
	IMAGE_DOS_HEADER *idh;
	if ( !ModuleHandle ) return FALSE;

	if ( IsBadReadPtr((void *)ModuleHandle, MIN_SIZE) )
	{
		ReadProcessMemory(hProc, (void *)ModuleHandle, buff, MIN_SIZE-1, NULL);
		idh = (IMAGE_DOS_HEADER *)buff;
	} else
		idh = (IMAGE_DOS_HEADER *)ModuleHandle;

	if ( idh->e_magic != IMAGE_DOS_SIGNATURE ) return FALSE;

	return (IMAGE_NT_HEADERS *)(RVA((DWORD )idh, (DWORD )idh->e_lfanew));
}


BOOL GetOptionalHeader(HANDLE hProc, DWORD ModuleHandle, IMAGE_OPTIONAL_HEADER *ioh)
{
	char buff[MIN_SIZE];	// stack room
	IMAGE_DOS_HEADER *idh;
	IMAGE_NT_HEADERS *nt;
	
	if ( !ModuleHandle ) return FALSE;
	ZeroMemory(buff, MIN_SIZE);

	if ( ReadProcessMemory(hProc, (void *)ModuleHandle, buff, MIN_SIZE, NULL) == 0 )
		return FALSE;
		
	idh = (IMAGE_DOS_HEADER *)buff;
	if ( idh->e_magic != IMAGE_DOS_SIGNATURE ) return FALSE;

	nt=(IMAGE_NT_HEADERS *)(RVA((DWORD )buff, (DWORD )idh->e_lfanew));

	memcpy(ioh, (IMAGE_OPTIONAL_HEADER *)&nt->OptionalHeader,
		sizeof(struct _IMAGE_OPTIONAL_HEADER) );

	return TRUE;
}


DWORD GetExportDirectory(
	HANDLE hProc, 
	DWORD ModuleHandle, 
	DWORD DataDir0,
	IMAGE_EXPORT_DIRECTORY *ied)
{
	char buff[MIN_SIZE];
	DWORD newread;

	newread = RVA(ModuleHandle, DataDir0);
	ReadProcessMemory(hProc, (void *)newread, buff, MIN_SIZE, NULL);

	memcpy(ied, (IMAGE_EXPORT_DIRECTORY *)buff, 
		sizeof(struct _IMAGE_EXPORT_DIRECTORY));

	return (DWORD )ied->NumberOfFunctions;
}
