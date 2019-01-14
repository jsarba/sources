; http://www.shellcode.com.ar
; (c) 1999 - 2006 ShellCode Research.
; *******************************************************
; Compile:
; nasmw -fwin32 -o mb.obj message.s
; 

; extern ExitProcess	; ExitProcess debe estar mapeado, porque carajo??
;	import ExitProcess kernel32.dll
;segment code use32 

segment .text
extern _exit
extern _FindLoadedDllAddress
extern _FindFunctionOnDllAddr
global _main

;..start
_main:
	push ebp
	mov dword ebp, esp
; *************************************************
; hacemos las busquedas y Preparamos todas las variables
; ******** Get address Kernel32.dll ************
	push dword Krnl_str
	call _FindLoadedDllAddress
	mov [Krnl_addr], eax		; store kernel32.dll addr
; ******** Search GetProcAddress address **********
	push dword [GPA_len]
	push dword GPA_str
	push dword [Krnl_addr]		; SearchFunction(u_long, char *, u_long);
	call _FindFunctionOnDllAddr
	mov [GPA_addr], eax			; stored
; ******** Search LoadLibraryA addresss ***********
	push dword [LL_len]
	push dword LL_str
	push dword [Krnl_addr]
	call _FindFunctionOnDllAddr
	mov [LL_addr], eax			; stored
; ******** LoadLibrary("user32.dll"); *************
	push dword User32_str
	call eax
	call .failtest
; ******** Get address MessageBoxA ****************
	push dword Message_str		; store onto stack: addr of "MessageBoxA"
	push dword eax				; and return handle of LoadLibrary
	mov ebx, [GPA_addr]			; restore GetProcAddress ADDRESS
	call ebx
	call .failtest
	mov [MB_addr], eax	
; ********* MessageBoxA ***************************
.mess:
	xor eax, eax
	inc eax
	push dword eax		
	push dword Topic_str		; Address of Topic
	push dword Bot_str			; Address of Content
	dec eax
	push dword eax
	mov ebx, [MB_addr]
	call ebx					; Call MessageBoxA
	;call mess
; *** EXIT PROCEDURE***
.Exit:
	xor dword eax, eax
	push dword eax
	call _exit
; *** CHECK RETURN VALUE ***
.failtest:
	test eax, eax
	jz .Exit
	ret

segment .bss
Krnl_addr	resd	1
GPA_addr	resd	1
LL_addr		resd	1
MB_addr		resd	1

segment .data
Krnl_str	db "kernel32.dll", 0
GPA_str		db "GetProcAddress", 0
LL_str		db "LoadLibraryA", 0
User32_str	db "user32.dll", 0
Message_str	db "MessageBoxA", 0
Topic_str	db "Win32 fr33 bush", 0
Bot_str		db "Frutas y verduras", 0
Exit_len	dd 11,0
LL_len		dd 12,0
Mess_len	dd 11,0
GPA_len		dd 14,0
