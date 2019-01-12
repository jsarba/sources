; Matias Sedalo <s0t4ipv6@shellcode.com.ar>
; http://www.shellcode.com.ar
; (c) 1999 - 2004 ShellCode Research.
;
; Mayo del 2004
; ****************************************************
; Compile:
; nasmw -fwin32 -o SearchFunction.obj SearchFunction.s

; SYNOPSIS
;	u_long FindFunctionOnDllAddr(u_long dll_base, char *Win32Api, long lenght);
; 
; DESCRIPTION
;	This function looks for the position of de Win32API in the memory segment where the DLL was loaded
; 	dll_base 	is the base memory position of the DLL, it can be obtained using FindLoadedDllAddress
;	Win32API	is the name of the routine exported(API) by the DLL 
;	lenght		is the lenght of Win32API string.
;
; RETURN VALUE
;	On sucess FindFunctionOnDllAdr, return the memory address of Win32API.
;	On error return zero.

global _FindFunctionOnDllAddr
segment .text

_FindFunctionOnDllAddr:
	push ebp				;
	mov dword ebp, esp		; Prologue
	sub esp, 0x14			; make room
	
	mov eax, [ebp+16]		; ebp+16 = lenght
	mov [ebp-0xc], eax		; Guardamos lenght of string
	mov eax, [ebp+12]		; ebp+12 = addr string
	mov [ebp-0x4], eax		; Guardemos la direccion del argumento
	mov eax, [ebp+8]		; ebp+8 = addr DLLBase
	mov [ebp-0x8], eax		; guardemos DllBase que despues la usamos

	cmp word [eax], 'MZ' 	; existe IMAGE_DOS_SIGNATURE en DOS-Stub
	jne .squit
	add eax, 0x3c			; de la struct _IMAGE_DOS_HEADER nos interesa "e_lfanew"
	mov eax, [eax]				
	add eax, [ebp-0x8]		; nos alineamos con DllBase
	cmp word [eax], 'PE'	; que sera nuestro offset hasta IMAGE_NT_SIGNATURE (4bytes)
	jne .squit				; seguidamente tendremos la struct IMAGE_FILE_HEADER (20bytes)

	add eax, 0x4+0x14+0x60	; pasemos a la struct IMAGE_OPTIONAL_HEADER precisamente
	mov esi, [eax]			; en la struct interna IMAGE_DATA_DIRECTORY
	add esi, [ebp-0x8]		; Align DLL Base with IMAGE_EXPORT_DIRECTORY
	add esi, 0x18			; hasta NumberOfNames
	lodsd
	push dword eax			; guardamos la cantidad de funciones(*)
	lodsd
	add eax, [ebp-0x8]		; Kernel Base align
	push dword eax			; Address of Functions
	lodsd 
	add eax, [ebp-0x8]		; KErnel Base Aling
	push dword eax			; Address of Names
	lodsd 
	add eax, [ebp-0x8]		; Kernl Base align
	push dword eax			; Address of Ordinals

	xor edx, edx
	mov esi, [ebp-0x4]		; char *Function
	mov eax, [esp+0x4]		; **AddressOfNames
.l00p:
	mov ecx, [ebp-0xc]		; ecx = strlen(Function)
	mov edi, [eax+edx*4]	; *AddressofNames[edx]
	add edi, [ebp-0x8]		; Kernel align
	inc edx					; # functions
	cld						; clear flag direction
	repe cmpsb				; 
	je .getaddr
	mov esi, [ebp-0x4]		; restore esi for compare
	cmp dword [esp+0xc], edx	; Compare edx con la cant de funciones(*)
	jne .l00p
.squit:
	xor eax, eax			; Not found | Kernel Base ERROR
	jmp .quit

.getaddr:	
	dec edx					; Puede estar desordenado en ..Ordinals..??
	pop eax					; **AddressOfNameOrdinals
	mov word dx, [eax+edx*2]	; word *AddressOfNameOrdinals[edx]
	pop eax					; 
	pop eax					; **AddressOfFunctions
	mov eax, [eax+edx*4]	; dword *AdressOfFunctions[edx]
	add eax, [ebp-0x8]		; we got the address on EAX.!
.quit:			
	mov dword esp, ebp		; leave
	pop dword ebp			; epilogue
	ret
