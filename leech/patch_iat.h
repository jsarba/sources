#ifndef _SANGUIJUELA_PATCH_IAT_H
#define _SANGUIJUELA_PATCH_IAT_H

// Declaracion de las funciones
DWORD	CallEntryPoint				(DWORD	, BYTE *		, DWORD	, HANDLE	, DWORD)	;
BOOL	ThreadReplicate			(HANDLE	, DWORD		, DWORD	, DWORD)				;
DWORD	SetImportDescriptor		(HANDLE	, DWORD		, DWORD	, ExpList *)		;
DWORD	ChangeImportDescriptor	(DWORD	, ExpList *	, char *)						;

#endif // _SANGUIJUELA_PATCH_IAT_H
