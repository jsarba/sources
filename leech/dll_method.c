#include "common.h"
#include "misc.h"
#include "t_core.h"

BOOL CompleteInjectApi(
	HANDLE hProc,
	DWORD ModuleHandle, 
	IMAGE_EXPORT_DIRECTORY ied, 
	ExpList *InjApi)
{
	DWORD FuncNum = ied.NumberOfFunctions;
	DWORD i, newread, AddressNames, Ordinals, Addresses;
	UCHAR *sym[FuncNum];
	DWORD address[FuncNum];
	USHORT ordinals[FuncNum];

	AddressNames = (RVA(ModuleHandle, (DWORD )ied.AddressOfNames));
	Ordinals = (RVA(ModuleHandle, (DWORD )ied.AddressOfNameOrdinals));
	Addresses = (RVA(ModuleHandle, (DWORD )ied.AddressOfFunctions));

	if ( !ReadProcessMemory(hProc, (void *)AddressNames, (UCHAR *)sym, FuncNum * sizeof(DWORD), NULL) ||
		 !ReadProcessMemory(hProc, (void *)Ordinals, ordinals, FuncNum * sizeof(DWORD), NULL) || 
		 !ReadProcessMemory(hProc, (void *)Addresses, address, FuncNum * sizeof(DWORD), NULL) )
		return FALSE;

	for (i=0; i < FuncNum; i++)
	{
		newread = (RVA(ModuleHandle, (DWORD )sym[i]));

		if ( !ReadProcessMemory(hProc, (void *)newread, InjApi->name, MAX_FUNC, NULL) )
			return FALSE;

		/* alineamos para descartar la "k" inicial del nombre de la funcion */
		memcpy(InjApi->name, InjApi->name+1, MAX_FUNC);
		InjApi->address = (DWORD )RVA(ModuleHandle, address[ordinals[i]]);
		printf("   %ld. %s [0x%lx]\n", i, InjApi->name, InjApi->address);
		InjApi++;
	}
	InjApi->address = 0;

	return TRUE;
}


DWORD LoadRemoteLibrary(DWORD pid, char *longLib)
{
	HANDLE hThread = 0;
	HANDLE hProc = 0;
	LPTHREAD_START_ROUTINE ThreadFunc;
	DWORD ret;
	void *p;
	char shortLib[MAX_FILE];

	(void )GetShortFileName(longLib, (char *)&shortLib);

	hProc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |
		PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);

	if ( hProc == NULL ) return -1;

	if ((p = VirtualAllocEx(hProc, 0, MAX_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE)) == 0)
		return 0;

	if ( !longLib || !WriteProcessMemory(hProc, (void *)p, (void *)longLib, MAX_FILE, NULL) )
		return 0;

	ThreadFunc = (LPTHREAD_START_ROUTINE )GetLibraryFunction(KERNEL, LoadLib);

	if ( (hThread = CreateRemoteThread(hProc, NULL, 0, ThreadFunc, p, 0, NULL)) == NULL )
		return 0;

	ret = WaitForSingleObject(hThread, INFINITE);

	switch (ret)
	{
		case WAIT_OBJECT_0:
			ret = GetRemoteHandle(pid, shortLib);
			break;
		default:
			ret = 0;
			break;
	}
	if ( p != 0 )
		VirtualFreeEx(hProc, p, 0, MEM_RELEASE);

	CloseHandle(hProc);
	CloseHandle(hThread);
	return ret;
}


DWORD InjectApiComplete(char *longLib, DWORD pid, ExpList **InjApi)
{
	HANDLE hProc = 0;
	DWORD ret = 0;
	DWORD OldModule;
	IMAGE_OPTIONAL_HEADER ImOh;
	IMAGE_EXPORT_DIRECTORY IeDir;
	char shortLib[MAX_FILE];

	(void )GetShortFileName(longLib, (char *)&shortLib);

	/* Si la dll ya fue cargada en el modulo entonces obtenemos el 
	 * HANDLE correspondiente y no hacemos el LoadRemotelibrary */
	if ( (OldModule = GetRemoteHandle(pid, shortLib)) == 0 )
	{
		if ( (OldModule = LoadRemoteLibrary(pid, longLib)) == 0 )
		{
			printf("! Can't inject library %s on process %ld\n", shortLib, pid);
			return -1;
		}
		printf("- Library %s injected at 0x%lx\n", shortLib, OldModule);
	} else
		printf("- Library %s found at 0x%lx\n", shortLib, OldModule);

	hProc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |
		PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);

	if ( hProc == NULL )
	{
		printf("! Can't open process %ld\n", pid);
		CloseHandle(hProc);
		return -1;
	}

	if ( !GetOptionalHeader(hProc, OldModule, &ImOh) )
	{
		printf("! Error GetOptionalHeader at 0x%lx\n", OldModule);
		CloseHandle(hProc);
		return -1;
	}

	ret = GetExportDirectory(hProc, OldModule, ImOh.DataDirectory[0].VirtualAddress, &IeDir);

	if ( ret == 0 )
	{
		printf("! Image export directory not found at 0x%lx\n", OldModule);
		CloseHandle(hProc);
		return -1;
	}

	*InjApi = GlobalAlloc(GMEM_FIXED , ((IeDir.NumberOfFunctions+1) * sizeof(struct _ExpList)) );

	if ( !CompleteInjectApi(hProc, OldModule, IeDir, *InjApi) )
		ret = -1;

	CloseHandle(hProc);
	return ret;
}
