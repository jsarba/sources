#include "common.h"
#include "misc.h"
#include "t_core.h"

DWORD CallEntryPoint(DWORD ThreadId, BYTE *pfunc, DWORD entrypoint, HANDLE hproc, DWORD hModule)
{
	// asm positions for thread replication
	const BYTE th_attach = 9;
	const BYTE h_module = 14;

	const BYTE call_indirect = 20;
	const BYTE call_direct = 74;

	const BYTE open_indirect = 50;
	const BYTE open_direct = 78;

	const BYTE close_indirect = 60;
	const BYTE close_direct = 82;

	FARPROC kOpenThread;
	FARPROC kOpenEvent;
	FARPROC kCloseHandle;
	HANDLE hThread = 0; 
	CONTEXT Context;
	DWORD offset, _th_asm, _th_len, tmp;
	BYTE array[MIN_SIZE];

	kOpenThread = (FARPROC )GetLibraryFunction(KERNEL, "OpenThread");
	hThread = (HANDLE )kOpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, 0, ThreadId);

	SuspendThread(hThread); 
	Context.ContextFlags = CONTEXT_CONTROL;
	GetThreadContext(hThread, &Context);

	ZeroMemory(array, MIN_SIZE);

	/* direccion y tama#o de la funcion de replicacion */
	_th_asm = (DWORD )&th_context;
	_th_len = strlen((char *)_th_asm) + PAD;

	if ( _th_len < MIN_SIZE ) // WARNING...
		memcpy(&array[0], (char *)_th_asm, _th_len);

	/* reason of replicate */
	tmp = (DWORD )DLL_THREAD_ATTACH;
	memcpy(&array[th_attach], &tmp, sizeof(DWORD));

	/* imageBase (HANDLE_MODULE) del nuevo modulo */
	memcpy(&array[h_module], &hModule, 4);

	/* direccion de EntryPoint del proceso */
	memcpy(&array[call_direct], &entrypoint, 4); 
	offset = (DWORD )(&pfunc[call_direct]);
	memcpy(&array[call_indirect], &offset, 4);

	/* direccion de OpenEvent */
	kOpenEvent = (FARPROC )GetLibraryFunction(KERNEL, "OpenEventA");
	memcpy(&array[open_direct], &kOpenEvent, 4);
	offset = (DWORD )(&pfunc[open_direct]);
	memcpy(&array[open_indirect], &offset, 4);

	/* direccion de CloseEvent */
	kCloseHandle = (FARPROC )GetLibraryFunction(KERNEL, "CloseHandle");
	memcpy(&array[close_direct], &kCloseHandle, 4);
	offset = (DWORD )(&pfunc[close_direct]);
	memcpy(&array[close_indirect], &offset, 4);
	
	/* resume del thread */
	//tmp = Context.Eip;
	//offset = (DWORD )(&pfunc[ret_direct]);
	//memcpy(&array[ret_indirect], &offset, 4); 
	//memcpy(&array[ret_direct], &tmp, 4);

	WriteProcessMemory(hproc, (void *)pfunc, (void *)array, MIN_SIZE, NULL);

	Context.Eip = (DWORD )&pfunc[0];
	SetThreadContext(hThread, &Context);
	ResumeThread(hThread);

	CloseHandle(hThread);
	return tmp;
}


BOOL ThreadReplicate(HANDLE hp, DWORD Pid, DWORD hMod, DWORD entry)
{
	FARPROC kThread32Next;
	HANDLE hEntry = 0;
	THREADENTRY32 thread;
	DWORD ret;
	BYTE *func;
	int first_thread=0;

	if ((func = VirtualAllocEx(hp, 0, MAX_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE)) == 0)
		return FALSE;

	printf("  >: common space: 0x%08lx\n", (long )func);

	kThread32Next = (FARPROC )GetLibraryFunction(KERNEL, "Thread32Next");
	hEntry = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	thread.dwSize = sizeof(THREADENTRY32);

	while ( kThread32Next(hEntry, &thread) != FALSE )
	{
		if ( thread.th32OwnerProcessID == Pid )
		{
			if ( first_thread > 0 )
			{
				ret = CallEntryPoint(thread.th32ThreadID, func, entry, hp, hMod);
				printf("  >: id: %08ld [0x%08lx]\n", thread.th32ThreadID, ret);
			} else 
				printf("  !: thread skiping %08ld\n", thread.th32ThreadID);

			first_thread++;
		}
	}
	return TRUE;
}


DWORD SetImportDescriptor(
	HANDLE hProc,
	DWORD RealModuleHandle,
	DWORD DataDir1,
	ExpList *InjApi)
{
	char th[MIN_SIZE];
	char buff[MIN_SIZE];
	char dllname[MAX_FILE];
	char apiname[MAX_FUNC];
	IMAGE_IMPORT_DESCRIPTOR *iid;
	IMAGE_THUNK_DATA *thunk;
	DWORD newread, IATaddress, *RealAddr;
	DWORD i, n, ret=0;

	newread = RVA(RealModuleHandle, DataDir1);
	ReadProcessMemory(hProc, (void *)newread, buff, MIN_SIZE, NULL);
	iid = (IMAGE_IMPORT_DESCRIPTOR *)buff;

	/* obtimizar esto... ya conozco la dll donde se encuentra la funcion
	 * que voy a cambiar, no deberia de recorrer todo */
	while ( iid->FirstThunk != 0 )
	{
		newread = RVA(RealModuleHandle, (DWORD )iid->Name);
		ReadProcessMemory(hProc, (void *)newread, dllname, MAX_FILE-1, NULL);
		LowerCase(dllname);
		printf("  >: %s\n", dllname);

		newread = RVA(RealModuleHandle, (DWORD )iid->OriginalFirstThunk);
		ReadProcessMemory(hProc, (void *)newread, th, MIN_SIZE-1, NULL);
		thunk = (IMAGE_THUNK_DATA *)th;

		i = 0;
		while ( thunk->u1.Function )
		{
			ZeroMemory(apiname, MAX_FUNC);
			newread = RVA(RealModuleHandle, (DWORD )thunk->u1.AddressOfData+2);
			ReadProcessMemory(hProc, (void *)newread, apiname, MAX_FUNC-1, NULL);

			IATaddress=RVA(RealModuleHandle, (DWORD )iid->FirstThunk) + (i*4);
			ReadProcessMemory(hProc, (void *)IATaddress, buff, sizeof(DWORD), NULL);
			RealAddr = (DWORD *)*((DWORD *)buff);

			n = 0;
			while( (InjApi+n)->address )
			{
				/* obtimizar el fucking codigo de todo esto */
				if ( strlen(apiname) < 1 ) break;
				if ( strncmp((InjApi+n)->name, apiname, strlen(apiname)) == 0 )
				{
					printf("   _imp__%s (IAT: 0x%lx) [0x%lx]", apiname, (DWORD )IATaddress, (DWORD )RealAddr);
					printf("<->[0x%lx]\n", (InjApi+n)->address);
					*(DWORD *)&buff[0] = (InjApi+n)->address;
					WriteProcessMemory(hProc, (void *)IATaddress, buff, sizeof(DWORD), NULL);
					ret++;
					break;
				}
				n++;
			} 
			i++;
			thunk++;
		}
		iid++;
	}
	return ret;
}


DWORD ChangeImportDescriptor(DWORD ProcId, ExpList *IApi, char *sLib)
{
	HANDLE hProc = 0;
	DWORD NewModule, DllModule = 0;
	DWORD EntryPoint = 0;
	DWORD changes = -1;
	IMAGE_OPTIONAL_HEADER ioh;
	
	NewModule = GetRemoteHandle(ProcId, NULL);
	printf("  . Base Module Handle: 0x%lx\n", NewModule);

	hProc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |
		PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, ProcId);

	if ( sLib ) // module of injected library
	{
		char shortLib[MAX_FILE];
		(void )GetShortFileName(sLib, (char *)&shortLib);

		DllModule = GetRemoteHandle(ProcId, shortLib);

		if ( !GetOptionalHeader(hProc, DllModule, &ioh) )
			return changes;

		DllModule = ioh.ImageBase; // Reasign DllModule
		printf("  . Dll ImageBase: 0x%lx\n", DllModule);

		EntryPoint = RVA(DllModule, ioh.AddressOfEntryPoint); // Confirm
		printf("  . Process Entry Point: 0x%lx\n", EntryPoint);
	}
	
	// now... for process module
	if ( !GetOptionalHeader(hProc, NewModule, &ioh) )
		return changes;

	if ( ioh.DataDirectory[1].VirtualAddress != 0 )
	{
		printf("  . IMAGE_IMPORT_DESCRIPTOR: 0x%lx\n", RVA(NewModule, ioh.DataDirectory[1].VirtualAddress));
		changes = SetImportDescriptor(hProc, NewModule, ioh.DataDirectory[1].VirtualAddress, IApi);
		if ( changes > 0 && DllModule != 0 )
		{
			printf("  . Threads calling entry point (0x%lx)...\n", EntryPoint);
			ThreadReplicate(hProc, ProcId, DllModule, EntryPoint);
		} else
			printf("  . Thread not replicate on PE_METHOD\n");
	}
		
	CloseHandle(hProc);
	return changes;
}
