.globl _kWriteFile, _kWriteFileEx, _kOpenFile, _kGetFileAttributesW, _kReadFile
.text

_kWriteFile:
	pushl %ebp
	movl %esp, %ebp
	subl $0x14, %esp /* WriteFile@20 stack */
	xorl %eax, %eax
	pushl %eax
	pushl $0x6e6f6974 /* Bill Information */
	pushl $0x616d726f
	pushl $0x666e4920
	pushl $0x6c6c6942
	movl %esp, %ebx	/* topic */
	pushl %eax
	pushl $0x2e6d6574 /* Read Only... */
	pushl $0x73797320
	pushl $0x656c6966
	pushl $0x20796c6e
	pushl $0x4f206461
	pushw $0x6552
	movl %esp, %ecx /* content */
	pushw %ax
	pushl %eax
	incl %eax
	pushl %eax
	pushl %ebx /* topic */
	pushl %ecx /* content */
	decl %eax
	pushl %eax
	movl $0x44444444, %eax /* MessageBoxA 73 */
	call *%eax
	leave
	ret $0x14 /* WriteFile@20 stack */

_kWriteFileEx:
	pushl %ebp
	movl %esp, %ebp
	subl $0x14, %esp /* WriteFileEx@20 stack */
	xorl %eax, %eax
	pushl %eax
	pushl $0x6e6f6974 /* Bill Information */
	pushl $0x616d726f
	pushl $0x666e4920
	pushl $0x6c6c6942
	movl %esp, %ebx	/* topic */
	pushl %eax
	pushl $0x2e6d6574 /* Read Only... */
	pushl $0x73797320
	pushl $0x656c6966
	pushl $0x20796c6e
	pushl $0x4f206461
	pushw $0x6552
	movl %esp, %ecx /* content */
	pushw %ax
	pushl %eax
	incl %eax
	pushl %eax
	pushl %ebx /* topic */
	pushl %ecx /* content */
	decl %eax
	pushl %eax
	movl $0x44444444, %eax /* MessageBoxA 73 */
	call *%eax
	leave
	ret $0x14 /* WriteFileEx@20 stack */


_kOpenFile:
	pushl %ebp
	movl %esp, %ebp
	subl $0x1c, %esp /* CreateFileA@28 stack */
	xorl %eax, %eax
	pushl %eax
	pushl $0x6e6f6974 /* Bill Information */
	pushl $0x616d726f
	pushl $0x666e4920
	pushl $0x6c6c6942
	movl %esp, %ebx	/* topic */
	pushl %eax
	pushl $0x2e6d6574 /* Read Only... */
	pushl $0x73797320
	pushl $0x656c6966
	pushl $0x20796c6e
	pushl $0x4f206461
	pushw $0x6552
	movl %esp, %ecx /* content */
	pushw %ax
	pushl %eax
	incl %eax
	pushl %eax
	pushl %ebx /* topic */
	pushl %ecx /* content */
	decl %eax
	pushl %eax
	movl $0x44444444, %eax /* MessageBoxA 73 */
	call *%eax
	leave
	ret $0x1c /* CreateFileA@28 stack */


_kGetFileAttributesW:
	pushl %ebp
	movl %esp, %ebp
	subl $0x1c, %esp /* CreateFileA@28 stack */
	xorl %eax, %eax
	pushl %eax
	pushl $0x6e6f6974 /* Bill Information */
	pushl $0x616d726f
	pushl $0x666e4920
	pushl $0x6c6c6942
	movl %esp, %ebx	/* topic */
	pushl %eax
	pushl $0x2e6d6574 /* Read Only... */
	pushl $0x73797320
	pushl $0x656c6966
	pushl $0x20796c6e
	pushl $0x4f206461
	pushw $0x6552
	movl %esp, %ecx /* content */
	pushw %ax
	pushl %eax
	incl %eax
	pushl %eax
	pushl %ebx /* topic */
	pushl %ecx /* content */
	decl %eax
	pushl %eax
	movl $0x44444444, %eax /* MessageBoxA 73 */
	call *%eax
	leave
	ret $0x1c /* CreateFileA@28 stack */

_kReadFile:
	pushl %ebp
	movl %esp, %ebp
	subl $0x1c, %esp /* CreateFileA@28 stack */
	xorl %eax, %eax
	pushl %eax
	pushl $0x6e6f6974 /* Bill Information */
	pushl $0x616d726f
	pushl $0x666e4920
	pushl $0x6c6c6942
	movl %esp, %ebx	/* topic */
	pushl %eax
	pushl $0x2e6d6574 /* Read Only... */
	pushl $0x73797320
	pushl $0x656c6966
	pushl $0x20796c6e
	pushl $0x4f206461
	pushw $0x6552
	movl %esp, %ecx /* content */
	pushw %ax
	pushl %eax
	incl %eax
	pushl %eax
	pushl %ebx /* topic */
	pushl %ecx /* content */
	decl %eax
	pushl %eax
	movl $0x44444444, %eax /* MessageBoxA 73 */
	call *%eax
	leave
	ret $0x1c /* CreateFileA@28 stack */

