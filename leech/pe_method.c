#include "common.h"
#include "t_core.h"
#include "misc.h"


// Nuevas funciones estaticas para el metodo PE
static ExpList StaticList[] =
{
	{ (DWORD )&kWriteFile, "WriteFile", "kernel32" },
	{ (DWORD )&kWriteFileEx, "WriteFileEx", "kernel32" },
	{ (DWORD )&kOpenFile, "OpenFile", "kernel32" },
	{ (DWORD )&kGetFileAttributesW, "GetFileAttributesW", "kernel32" },
	{ (DWORD )&kReadFile, "ReadFile", "kernel32" },
	{ 0 },
	// agregar aqui las nuevas funciones
};

/* Solo para PE_METHOD ya que el codigo puro inyectado en memoria
 * no contendra la direccion de memoria real del MessageBoxA */
void AsignDinamicPoint(HANDLE proc, DWORD addr)
{
	// source insertion
	const char *act_name = "MessageBoxA";
	const BYTE msg_box = 73;
	
	DWORD realpoint;
	HMODULE ll;

	ll = LoadLibraryA(USER32);

	/* act_name constante de MessageBoxA */
	realpoint = (DWORD )GetProcAddress(ll, act_name);
	/* Posicion de mov $MessageBoxA, %eax */
	addr += msg_box;
	/* la direccion de MessageBoxA */	
	WriteProcessMemory(proc, (void *)addr, (void *)&realpoint, sizeof(DWORD), NULL);

	FreeLibrary(ll);
	return;
}


DWORD GetSectionFreeMemory(HANDLE hProc, DWORD hMod, DWORD *f)
{
	IMAGE_NT_HEADERS *nt;
	IMAGE_SECTION_HEADER *ish;
	DWORD SectionAlign, SizeOfRawData;
	DWORD VirtualSize, VirtualAddress;

	nt = GetNtHeaders(hProc, hMod);
	if ( section + 1 > nt->FileHeader.NumberOfSections )
		return 0;

	ish = GetSectionHeader(nt, section);

	SectionAlign = nt->OptionalHeader.SectionAlignment;
	VirtualAddress = RVA(hMod, (DWORD )(ish->VirtualAddress & ~(SectionAlign -1)));
	SizeOfRawData = ish->SizeOfRawData;

	VirtualSize = ish->Misc.VirtualSize;

	if ( VirtualSize > SizeOfRawData ) // ????
		*f = (( VirtualSize + SectionAlign -1 ) & ~(SectionAlign -1)) - VirtualSize - PAD;
	else
		*f = 0 ; //(( SizeOfRawData + SectionAlign -1 ) & ~(SectionAlign -1)) - SizeOfRawData - PAD;

	section++;

	return VirtualAddress + VirtualSize;
}


BOOL CompleteInjApiStruct(DWORD pos, DWORD addr, ExpList *InjApi)
{
	(InjApi+pos)->address = addr;

	memcpy(&(InjApi+pos)->name, &StaticList[pos].name, MAX_FILE);
	memcpy(&(InjApi+pos)->ondll, &StaticList[pos].ondll, MAX_FILE);
	
	return TRUE;
}


DWORD StaticInjectApiComplete(DWORD pid, ExpList **InjApi)
{
	DWORD i, count, wrote;
	DWORD off = 0;
	UCHAR *src;
	HANDLE hProcess = 0;

	hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |
		PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);

	/* Cantidad maxima de nuevas funciones */
	count = sizeof(StaticList) / sizeof(struct _ExpList);

	*InjApi = GlobalAlloc(GMEM_FIXED, (count * sizeof(struct _ExpList)) );

	/* el algoritmo solo podra escribir tantas funciones como espacios libres
	 * halla. TODO: Modificarlo para concatenar funciones en un mismo segmento
	 * de memoria considerando los espacios libres encontrados */
	for (i=0; i < count; i++)	
	{
		DWORD lenght, address;
		FreeMem_l *tmp;

		tmp = head_ptr;
		do
		{
			src = (UCHAR *)StaticList[i].address;
			lenght = tmp->item.mem_len;
			address = tmp->item.mem_addr;

			/* escribimos la nueva funcion en el primer espacio libre 
			 * priorizando el modulo principal del proceso (el mismo)
			 * Si no hay exito entonces contamos el error */
			if ( WriteProcessMemory(hProcess, (void *)address, (void *)src, lenght, &wrote) 
					&& ( wrote == lenght ) )
			{
				printf("  . Wrote 0x%lx(%s) at 0x%lx\n", (DWORD )src, StaticList[i].name, address);
				/* Cambiemos el valor de MessageBoxA en tiempo real 
			 	* segun la version y service pack de windows.
				* La modificacion la realizamos directamente en la posicion
				* final de la funcion (segmento del proceso) */
				(void )AsignDinamicPoint(hProcess, (DWORD )address);

				/* off deberia terminar con el mismo valor que count 
				 * la funcion deberia dar siempre TRUE */
				off += CompleteInjApiStruct(i, address, *InjApi);

				head_ptr = tmp->next; 	// por exito, marcamos la siguiente direccion(tmp)
				break;					// como comienzo de la siguiente funcion(i)
			} else 
				tmp = tmp->next;		// siguiente direccion libre
		} while (tmp);
		
	}

	CloseHandle(hProcess);
	return off;
}


DWORD CompleteFreeMemList(DWORD pid)
{
	FARPROC kModule32Next; 
	MODULEENTRY32 ModEntry;
	HANDLE hEntry=0;
	HANDLE hProcess=0;
	DWORD ProxyAddr, FreeBytes, hModule;
	DWORD found=0;

	kModule32Next = (FARPROC )GetLibraryFunction(KERNEL, "Module32Next");
	hEntry = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
	ModEntry.dwSize = sizeof(MODULEENTRY32);

	hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |
			PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);

	if ( hProcess == NULL ) return -1;

	/* Todos sus modulos tienen asignada memoria compartida accesible por el proceso */
	while( kModule32Next(hEntry, &ModEntry) != FALSE )
	{
		hModule = (DWORD )ModEntry.hModule;
		while ( (ProxyAddr = GetSectionFreeMemory(hProcess, hModule, &FreeBytes)) != 0 )
		{
			if ( FreeBytes )
			{
				// printf("- Module: %s (0x%lx)\n", ModEntry.szModule, hModule);
				/* agregamos la direccion y espacio libre encontrado
				 * no nos importa el modulo que tiene el espacio libre */
				if ( Add_MemItem(ProxyAddr, FreeBytes) == 0 ) return -1;
				found++;
			}
		}
		section = 0; /* liberemos la cantidad de secciones */
	}
		
	CloseHandle(hProcess);
	/* devolvemos la cantidad de espacios encontrados para 
	 * pode avanzar el puntero de lista para el siguiente proceso */ 
	return found;
}
