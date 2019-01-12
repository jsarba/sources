/*
 *
 * compile
 * gcc -c lib-krnl.c
 * gcc -Wall -shared -o lib-krnl.dll lib-krnl.o
 */

#include <windows.h>
#include <winbase.h>

#define AUTHOR "Bill gates: Information"
#define LEYEND "You're not authorized for manipulate files"

BOOL WINAPI CommonAction()
{
	MessageBoxA(NULL, LEYEND, AUTHOR, MB_OK	| MB_ICONWARNING);
	return FALSE;
}

/*
 *
*/
BOOL DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID fImpLoad)
{
	HINSTANCE hinst;
	switch (fdwReason) 
	{ 
		case DLL_PROCESS_ATTACH: 
			(void )CommonAction();
		// The DLL is being mapped into the 
		// process's address space. 
		break; 
 
	case DLL_THREAD_ATTACH: 
			(void )CommonAction();
		// A thread is being created. 
		break; 
 
	case DLL_THREAD_DETACH: 
		// A thread is exiting cleanly. 
		break; 
 
	case DLL_PROCESS_DETACH: 
		// The DLL is being unmapped from the 
		// process's address space. 
		break; 
  } 
  hinst = hinstDll;
  return TRUE; 
}



BOOL __declspec(dllexport) WINAPI kWriteFile(
    HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
   )
{
	return CommonAction();
}


BOOL __declspec(dllexport) WINAPI kWriteFileEx(
	HANDLE hFile, 
	LPCVOID lpBuffer, 
	DWORD nNumberOfBytesToWrite,
	LPOVERLAPPED lpOverlapped, 
	LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
	)
{
	return CommonAction();
}

	

HANDLE __declspec(dllexport) WINAPI kCreateFileA(
    LPCTSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDistribution,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
   )
{
	return (HANDLE )CommonAction();
}


HANDLE __declspec(dllexport) WINAPI kCreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDistribution,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
   )
{
	return (HANDLE )CommonAction();
}


BOOL __declspec(dllexport) WINAPI kReadFile(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,	
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
   )
{
	return CommonAction();
}

HFILE __declspec(dllexport) WINAPI kOpenFile(
    LPCSTR lpFileName,
    LPOFSTRUCT lpReOpenBuff,
    UINT uStyle	
   )
{
	return (HFILE )CommonAction();
}

DWORD __declspec(dllexport) WINAPI kGetFileAttributesA(
    LPCTSTR lpFileName 
   )
{
	(void )CommonAction();
	return 0xFFFFFFFF;
}

 
DWORD __declspec(dllexport) WINAPI kGetFileAttributesW(
    LPCWSTR lpFileName 
   )
{
	(void )CommonAction();
	return 0xFFFFFFFF;
}
