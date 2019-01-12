; Matias Sedalo <s0t4ipv6@shellcode.com.ar>
; http://www.shellcode.com.ar
; (c) 1999 - 2004 ShellCode Research.
;
; Mayo del 2004
; ******************************************
; Compile:
; nasmw -fwin32 -o SearchDLL.obj SearchDLL.s

; SYNOPSIS
; 	u_long FindLoadedDllAddress(char *DLLName);
;
; DESCRIPTION
;	This function looks for the memory position where the DLL was loaded.
;
; RETURN VALUE
;   On success, FindLoadedDllAddress return the memory address.
;	On error return zero  

global _FindLoadedDllAddress
segment .text
; **************************************************************************
; char *eax="kernel32.dll";
; Cuando uso 'esp' es para denotar la estabilidad del stack
; **************************************************************************
_FindLoadedDllAddress:
	push ebp
	mov dword ebp, esp
	mov dword eax, [esp+8]	; argumento (string)
	test eax, eax
	jz .r0
	push dword eax		; Name of dll to search
; ****************************
; Esta parte no va para Win98 (hay que buscar esto)
	mov eax, [fs:0x30]	; Puntero a PEB (Ej: 0x7ffdf000, porque aparece en %ebx tambien?)
	mov eax, [eax+0xc]	; Miembro de la estructura PEB, PVOID ListDlls 
						; 0xc del puntero a PEB
; ****************************
	add eax, 0xc 		; Ahora en eax tengo otro puntero a la struct MODULELISTHEADER
						; Nos interesa la direccion de LoadOrder a 0xc bytes.
	mov eax, [eax]
; **************************************************************************
; aca empezamos la busqueda en memoria de la dll
; podria ser (ntdll.dll, kernel32.dll, msvcrt.dll)
; **************************************************************************
.bucle:
	mov ebx, [eax] 		; en ebx tengo direccion de next
	add eax, 0x30 		; puntero al nombre de la dll en unicode
	mov edx, [eax]		; tengo en esi la direccion del nombre de la dll
	mov edi, [esp]		; pop dword edi 		; addr of argument
	push dword eax		; store register
	push dword ebx		
; **************************************************************************
; buscamos el string en unicode
; no puedo tocar eax y ebx
; en edi ya tengo la direccion del nombre en unicode
; **************************************************************************
	xor eax, eax
	xor ebx, ebx
 .b0:
	mov byte bl, [edi]
	test bl, bl
	jz .b1
	mov byte al, [edx]
	or byte al, 0x20	; uppercase
	or byte bl, 0x20	; uppercase
	cmp byte al, bl
	jne .next
	inc edi
	inc edx
	inc edx
	jmp .b0
; **************************************************************************
 .b1:pop dword ebx
	pop dword eax
	sub eax, 0x18		; ULONG ImageBase de la estructura tagMODULEITEM
	mov eax, [eax]		; I got the address of Library
 .r0:
	mov dword esp, ebp  ; epilogue
	pop dword ebp		; 
	ret					; RETURN
; **************************************************************************
; nos movemos al siquiente unicode
; **************************************************************************
.next:
 	pop dword ebx		; necesito trabajar mas sobre esto!!!!
	pop dword eax		; necesito tiempo.............
	mov eax, ebx                                                                
	mov ecx, [eax+0x18]	; detectamos el final de la estructura
	jecxz .r1			; comprobamos la direccion
	jmp .bucle  
.r1:
	xor eax, eax		; eax = 0 ; DLL not found
	jmp .r0
