#include "common.h"

FreeMem_l *head_ptr = NULL;
FreeMem_l *tail_ptr = NULL;


void HowUse(char **argv)
{
	printf("usage: %s [Process.Name] <path\\dll>\n", argv[0]);
	exit(0);
}


inline void LowerCase(char *s)
{
	size_t i;
	for (i=0; i < strlen(s); i++) s[i] |= 0x20;
}

BOOL Add_MemItem(DWORD addr, DWORD len)
{
	FreeMem_l *newnode;

	if ( (newnode = (FreeMem_l *)LocalAlloc(LPTR, sizeof(FreeMem_l))) == NULL )
		return FALSE;

	newnode->item.mem_addr = addr;
	newnode->item.mem_len = len;
	newnode->next = NULL;

	if ( head_ptr == NULL )
		head_ptr = newnode;
	else
		tail_ptr->next = newnode;

	tail_ptr = newnode;

	return TRUE;
}

void GetShortFileName(char *src, char *dst)
{
	DWORD i;
	DWORD len = strlen(src);

	for (i=0; i < len; i++)
		if ( src[i] == '\\' ) 
			memcpy(dst, src+i+1, len - i); // MAX_FILE
}


BOOL FileExist(char *filename)
{
	DWORD attr;

	attr = GetFileAttributes(filename);

	if ( attr == 0xffffffff || attr == FILE_ATTRIBUTE_DIRECTORY )
		return FALSE;
	else
		return TRUE;
}
