#ifndef _SANGUIJUELA_DLL_METHOD_H
#define _SANGUIJUELA_DLL_METHOD_H

BOOL	CompleteInjectApi	(HANDLE, DWORD, IMAGE_EXPORT_DIRECTORY, ExpList *);
DWORD LoadRemoteLibrary	(DWORD, char *);
DWORD InjectApiComplete	(char*, DWORD, ExpList **);

#endif // _SANGUIJUELA_DLL_METHOD_H
