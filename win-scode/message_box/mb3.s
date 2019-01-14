; stpv6@shellcode.com.ar
; *******************************************************
; nasmw -fwin32 -o main.obj main.s
; For 32 bits
; alink -oPE mb2.obj -entry _main

; For 16 bits
; gcc -Wall -o main.exe FindLoadedDllAddress.obj 
; and adjust code below

segment .text	 USE32
global _main
_main:
; ******** LoadLibrary("user32.dll"); *************
;	call _load_user32
; ******** GetProcAddress("MessageBoxA"); *********
;	call _get_messagebox
; ********* MessageBoxA ***************************

;_load_user32:
; ******** LoadLibrary("User32.dll") ***********
	push dword ebp
	mov ebp, esp
	xor eax, eax
	mov byte al, 0x16
	sub esp, eax
	xor eax, eax
	push dword eax
	push dword 6c6c642eh 	;"User32.dll"
	push dword 32337265h
	push word 7375h
	push dword esp
	mov eax, 7C801D77h	;[LL_addr]
	call eax
	leave
;	ret
; ******** GetProcAddres("MessageBoxA") ****************
;_get_messagebox:
	push dword ebp
	mov ebp, esp
	xor ecx, ecx
	mov byte cl, 0x20
	sub esp, ecx
	xor ecx, ecx
	push dword ecx
	push word cx
	push byte 0x41
	push word 0x786f
	push dword 0x42656761
	push dword 0x7373654d
	mov ebx, esp
	push dword ecx
	push dword ebx
	push dword eax		; and return handle of LoadLibrary
	mov ebx, 7c80ada0h	; [GPA_addr]
	call ebx
	leave
;	ret
	mov esi, eax
	xor eax, eax
	push dword eax
	push dword 0x6e6f6974 ; Bill Information 
	push dword 0x616d726f
	push dword 0x666e4920
	push dword 0x6c6c6942
	mov ebx, esp
	push dword eax
	push dword 0x2e6d6574 ;
	push dword 0x73797320
	push dword 0x656c6966
	push dword 0x20796c6e
	push dword 0x4f206461
	push word 0x6552
	mov ecx, esp
	push word ax
	push dword eax
	inc eax
	push dword eax
	push dword ebx ;
	push dword ecx ; content */
	dec eax
	push dword eax
	mov eax, esi;MessageBoxA
	call eax
; *** EXIT ***
_fin:
	push dword eax
	mov ebx, 7C81CDDAh	;[Exit_addr]	
	call ebx	
