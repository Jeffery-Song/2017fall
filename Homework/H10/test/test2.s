	.file	"test2.c"
	.globl	k
	.data
	.align 4
	.type	k, @object
	.size	k, 4
k:
	.long	100
	.globl	p
	.align 8
	.type	p, @object
	.size	p, 8
p:
	.quad	k
	.section	.rodata
.LC0:
	.string	"%d, %d\n"
.LC1:
	.string	"%d\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	p(%rip), %rax
	movq	%rax, %rdx
	movl	$k, %esi
	movl	$.LC0, %edi
	movl	$0, %eax
	call	printf
	movq	buf(%rip), %rax
	movq	%rax, %rsi
	movl	$.LC1, %edi
	movl	$0, %eax
	call	printf
	movl	$0, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.4) 5.4.0 20160609"
	.section	.note.GNU-stack,"",@progbits
