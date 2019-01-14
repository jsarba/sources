; s0t4ipv6@shellcode.com.ar
; *******************************************************
; nasmw -fwin32 -o main.obj main.s
; nasmw -fwin32 -o FindLoadedDllAddress.obj FindLoadedDllAddress.s
; nasmw -fwin32 -o FindFunctionOnDllAddr.obj FindFunctionOnDllAddr.s
; For 32 bits
; alink -oPE main.obj FindFunctionOnDllAddr.obj FindLoadedDllAddress.obj win32.lib -entry _main
; For 16 bits
; gcc -Wall -o main.exe main.obj FindFunctionOnDllAddr.obj FindLoadedDllAddress.obj 
; and adjust code below

segment .data	 USE32
Krnl_str	db "kernel32.dll", 0
GPA_str		db "GetProcAddress", 0
Exit_str	db "ExitProcess", 0
LL_str		db "LoadLibraryA", 0
User32_str	db "user32.dll", 0
Message_str	db "MessageBoxA", 0
Topic_str	db "Win32 t3st free", 0
Bot_str		db "Frutas y verduras", 0
GPA_len		dd 14,0
Exit_len	dd 11,0
LL_len		dd 12,0
Mess_len	dd 11,0

segment .bss	 USE32
Krnl_addr	resd	1
GPA_addr	resd	1
Exit_addr	resd	1
LL_addr		resd	1
MB_addr		resd	1

segment .text	 USE32
global _main

; extern ExitProcess			; En 32 bits ExitProcess debe estar mapeado
					; es decir que es utilizada esta importacion
extern _FindLoadedDllAddress
extern _FindFunctionOnDllAddr

_main:
; *************************************************
; hacemos las busquedas y Preparamos todas las variables
; ******** Get address Kernel32.dll ************
	push dword Krnl_str
	call _FindLoadedDllAddress
	mov [Krnl_addr], eax	; store kernel32.dll addr
; ******** Search ExitProcess address **********
	push dword [Exit_len]
	push dword Exit_str
	push dword [Krnl_addr] 	; FindFunctionOnDllAddr(char , u_long *, long);
	call _FindFunctionOnDllAddr
	mov [Exit_addr], eax		; stored
; ******** Search GetProcAddress address **********
	push dword [GPA_len]
	push dword GPA_str
	push dword [Krnl_addr] ; FindFunctionOnDllAddr(char *, u_long);
	call _FindFunctionOnDllAddr
	mov [GPA_addr], eax		; stored
; ******** Search LoadLibraryA addresss ***********
	push dword [LL_len]
	push dword LL_str
	push dword [Krnl_addr]
	call _FindFunctionOnDllAddr
	mov [LL_addr], eax		; stored
; ******** LoadLibrary("user32.dll"); *************
	push dword User32_str
	call eax
	call _failtest
;	mov [User32_addr], eax	; stored mapped user32.dll addr
							; Not defined
; ******** Get address MessageBoxA ****************
	push dword Message_str	; store onto stack: addr of "MessageBoxA"
	push dword eax			; and return handle of LoadLibrary
	mov ebx, [GPA_addr]		; restore GetProcAddress ADDRESS
	call ebx
	call _failtest
	mov [MB_addr], eax	
; ********* MessageBoxA ***************************
mess:
	xor eax, eax
	inc eax
	push dword eax		
	push dword Topic_str	; Address of Topic
	push dword Bot_str		; Address of Content
	dec eax
	push dword eax
	mov ebx, [MB_addr]
	call ebx				; Call MessageBoxA
;	call mess

; *** EXIT ***
_fin:
	push dword eax
	mov ebx, [Exit_addr]	; only for 16 bit gcc compiler
	call ebx				; <>
;	call ExitProcess		; ExitProcess debera estar mapeado antes que nada
							;

; *** CHECK RETURN VALUE ***
_failtest:
	test eax, eax
	jz _fin
	ret
