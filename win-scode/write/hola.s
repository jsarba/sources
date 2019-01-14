.globl main
main:
xorl %eax,%eax
xorl %ebx,%ebx
movb $0x4,%al
movb $0x1,%bl
pushl $0x6f646e75
pushl $0x6d20616c
pushw $0x6f68
movl %esp,%ecx
xorl %edx,%edx
movb $0xa,%dl
int $0x80
movb $0x1,%al
movb $0x1,%bl
int $0x80
