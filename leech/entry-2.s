.text
.globl _th_context
_th_context:
	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushf
	xorl %eax, %eax	/* 2b */
	pushl %eax
	pushl $0x33333333 /* 9 th_attach */
	pushl $0x33333333 /* 14 h_module */
	call *0x33333333 /* 20 call_indirect */

	xorl %eax, %eax	/* 2b */
	pushl %eax
	pushl $0x68737562	/* nullbush */
	pushl $0x6c6c756e
	movl %esp, %ebx	/* 2b */
	pushl %eax
	pushl %ebx	/* *ptr nullbush */
	pushl %eax
	pushw $0x1f /* EVENT_ALL_ACCESS */ /* 3b */
	pushw $0x3 	/* 3b */
	call *0x33333333	/* 50 open_indirect */
	xorl %ebx, %ebx
	pushl %ebx
	pushl %eax
	call *0x33333333	/* 60 close_indirect */
	addl $48, %esp /* realign stack 3b */ 
	popf
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	ret
	nop
	.long 0x41414141 /* 74 call_direct */
	.long 0x41414141 /* 78 open_direct */
	.long 0x41414141 /* 82 close_direct */
	nop
