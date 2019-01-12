#include "common.h"

#ifndef _SANGUIJUELA_T_CORE_H
#define _SANGUIJUELA_T_CORE_H

FARPROC						GetLibraryFunction	(char*, char*);
DWORD							GetTotalProcess		(void);
DWORD							GetRemoteHandle		(DWORD, char*);
DWORD							CompleteGralProcess	(char*, DWORD*, int, BOOL);
IMAGE_SECTION_HEADER*	GetSectionHeader		(IMAGE_NT_HEADERS*, BYTE);
IMAGE_NT_HEADERS*			GetNtHeaders			(HANDLE, DWORD);
BOOL							GetOptionalHeader		(HANDLE, DWORD, IMAGE_OPTIONAL_HEADER*);
DWORD							GetExportDirectory	(HANDLE, DWORD, DWORD, IMAGE_EXPORT_DIRECTORY*);

#endif // _SANGUIJUELA_T_CORE_H
