/*
 * gcc -c static_api.s
 * gcc -c entry-2.s
 * gcc -Wall -o total.exe total.c entry-2.o static_api.o
 *
 */
#include "common.h"
#include "misc.h"

#include "t_core.h"
#include "pe_method.h"
#include "dll_method.h"
#include "patch_iat.h"

static FreeMem_l **fMem;
int section = 0;

int main(int argc, char **argv)
{
	int i, method = PE_METHOD;
	DWORD MaxProcess = GetTotalProcess();
	DWORD RealProcess, tmpProc;
	DWORD api_count, sum = 0;
	ExpList *InjectApi;
	static DWORD *ProcessList;

	if ( argc < 2 ) HowUse(argv);
	if ( argv[2] )
	{
		if ( !FileExist(argv[2]) )
		{
			printf("! File %s not found\n", argv[2]);
			HowUse(argv);
		}
		method = DLL_METHOD;
	}

	/* Alocamos memoria para la cantidad maxima de procesos */
	ProcessList = LocalAlloc(LPTR, (MaxProcess * sizeof(DWORD)) );
	if ( ProcessList == NULL ) exit(-1);

	/* CharLowerBuff() ?no? */
	LowerCase(argv[1]); /* cambiamos el nombre del proceso a minusculas */

	/* Completamos la estructura de procesos y asignamos la cantidad
	 * real de procesos encontrados con el nombre argv[1] */
	RealProcess = CompleteGralProcess(argv[1], ProcessList, method, PATCH_EXP);
	
	for ( i=0; i < RealProcess; i++ )
	{
		api_count = 0;
		tmpProc = ProcessList[i];
		printf("\n- Process ID: %ld\n", tmpProc);

		/* Procedimientos particulares al metodo por cada proceso */
		switch ( method )
		{
			case PE_METHOD:
				head_ptr = NULL;
				/* Lista de memoria por proceso */
				fMem = &head_ptr + sum;
				sum = CompleteFreeMemList(tmpProc);
				printf("  . Add %ld free memory positions\n", sum);

				if ( sum == -1 ) sum = 0; /* Error */
				else 	/* Escribimos en la memoria libre encontrada, priorizando el
				 		* modulo principal, y completamos InjectApi */
					api_count = StaticInjectApiComplete(tmpProc, &InjectApi);

				break;
			case DLL_METHOD:		
				fMem = NULL; /* No utilizamos la lista */

				/* Recorremos IMAGE_EXPORT_DIRECTORY buscamos las funciones
				 * exportadas y completamos InjectApi. Alocamos memoria en el 
				 * mismo procedimiento. Revisar GlobalAlloc(GMEM_FIXED) 
				 * -OpenProcess
				 * -LoadRemoteLibrary
				 * -GetExportDirectory */
				api_count = InjectApiComplete(argv[2], tmpProc, &InjectApi);

				break;
		}
		/* Procedimientos generales independientes del metodo */

		/* InjectApi contiene las funciones que seran reemplazas y en api_count
		 * tenemos la cantidad */
		if ( api_count > 0 )
		{
			if ( ChangeImportDescriptor(tmpProc, InjectApi, argv[2]) > 0 )
				printf("- Hooking successfull\n");					
		}	
		/* Por cada proceso alocamos memoria y completamos la estructura InjectApi;
	 	* aunque la direccion entre procesos sea la misma, el segmento es otro */
		LocalFree(InjectApi);
	}

	LocalFree(ProcessList);
	exit(0);
}
