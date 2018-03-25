# HW1

宋小牛 PB15000301

[TOC]

##预处理

以一道简单的oj题为例

> 整数a1，a2，a3。已知0 <= a1, a2, a3 <= n，而且a1 + a2是2的倍数，a2 + a3是3的倍数， a1 + a2 + a3是5的倍数。你的任务是找到一组a1，a2，a3，使得a1 + a2 + a3最大。


代码实现如下，默认n值为100

```c
//#define _use_stdin_
#ifdef _use_stdin_
#include <stdio.h>
#else
#define num 100
#endif
int main() {
    int a1, a2, a3, n, h = 0;
#ifdef _use_stdin_
    int num;
    scanf("%d%d%d", &num);
#endif
    n = num;
    for (a2 = n; a2 >= 0; a2--) {
        for (a1 = n - (n + a2) % 2; a1 >= 0; a1 -= 2) {
            for (a3 = n - (n + a2) % 3; a3 >= 0; a3 -= 3) {
                if (a1 + a2 + a3 <= h) break;
                if ((a1 + a2 + a3) % 5 == 0) h = a1 + a2 + a3;
            }
        }
    }
    return 0;
}
```

执行预处理

```shell
cpp main.c -o main.i
```

得到预处理后的`main.i`文件

```c
# 1 "simple.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 1 "<command-line>" 2
# 1 "simple.c"
# 9 "simple.c"
int main() {
    int a1, a2, a3, n, h = 0;




    n = 100;
    for (a2 = n; a2 >= 0; a2--) {
        for (a1 = n - (n + a2) % 2; a1 >= 0; a1 -= 2) {
            for (a3 = n - (n + a2) % 3; a3 >= 0; a3 -= 3) {
                if (a1 + a2 + a3 <= h) break;
                if ((a1 + a2 + a3) % 5 == 0) h = a1 + a2 + a3;
            }
        }
    }
    return 0;
}
```

可见伪指令如预期已被处理完毕

将被注释的`#define _use_stdin_`补回后生成的`sample.i`内包含了`stdio.h`的全部内容，内容过长在此不放出。

## 汇编码

```shell
clang -S sample.c -m32
```

生成`sample.s`文件如下(以将默认生成的注释删除)

```assembly
	.text
	.file	"simple.c"
	.globl	main
	.align	16, 0x90
	.type	main,@function
main:                                   # @main
# BB#0:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	subl	$36, %esp			# alloc space for local variable
	movl	$0, -8(%ebp)
	movl	$0, -28(%ebp)		# h = 0
	movl	$100, -24(%ebp)		# n = 100
	movl	-24(%ebp), %eax		# %eax = n
	movl	%eax, -16(%ebp)		# a2 = %eax, mov (mem), (mem) is not allowed
.LBB0_1:                                
                                       
                                        
	cmpl	$0, -16(%ebp)		# compare a2 to 0
	jl	.LBB0_16				# jump to the end if 0 is larger than a2
# BB#2:                                
	movl	$2, %eax			# %eax = 2
	movl	-24(%ebp), %ecx		# %ecx = n
	movl	-24(%ebp), %edx		# %edx = n
	addl	-16(%ebp), %edx		# %edx = a2 + %edx = a2 + n
	movl	%eax, -32(%ebp)         
								# -32(%ebp) = 2
	movl	%edx, %eax			# %eax = a2 + n
	cltd
	movl	-32(%ebp), %esi         
								# %esi = 2
	idivl	%esi				# %edx = %eax % %esi = (n + a2) % 2
	subl	%edx, %ecx			# %ecx = %ecx - %edx = n - (n + a2) % 2
	movl	%ecx, -12(%ebp)		# a1 = n - (n + a 2) % 2
.LBB0_3:                                
                                        
                                        
	cmpl	$0, -12(%ebp)		# compare a1 to 0
	jl	.LBB0_14				# if 0 > a1, jump to the end of the second 'for'
# BB#4:                                 
	movl	$3, %eax			# %eax = 3
	movl	-24(%ebp), %ecx		# %ecx = n
	movl	-24(%ebp), %edx		# %edx = n
	addl	-16(%ebp), %edx		# %edx = a2 + n
	movl	%eax, -36(%ebp)         
								# -36(%ebp) = 3
	movl	%edx, %eax			# %eax = a2 + n
	cltd
	movl	-36(%ebp), %esi         
								# %esi = 3
	idivl	%esi				# %edx = %eax % %esi = (n + a2) % 3
	subl	%edx, %ecx			# %ecx = %ecx - %edx = n - (n + a2) % 3
	movl	%ecx, -20(%ebp)		# a3 = n - (n + a2) % 3
.LBB0_5:                               
                                        
                                        
	cmpl	$0, -20(%ebp)		# if 0 > a3, jump to the end of the third 'for'
	jl	.LBB0_12
# BB#6:                                 
	movl	-12(%ebp), %eax		# %eax = a1
	addl	-16(%ebp), %eax		# %eax = a1 + a2
	addl	-20(%ebp), %eax		# %eax = a1 + a2 + a3
	cmpl	-28(%ebp), %eax		# compare a1 + a2 + a3 to h
	jg	.LBB0_8					# if a1 + a2 + a3 > h, pass the break cmd next
# BB#7:                                 
	jmp	.LBB0_12				# break, jump to the end of the third 'for'
.LBB0_8:                               
	movl	$5, %eax
	movl	-12(%ebp), %ecx
	addl	-16(%ebp), %ecx
	addl	-20(%ebp), %ecx		# %ecx = a1 + a2 + a3
	movl	%eax, -40(%ebp)         
	movl	%ecx, %eax			# %eax = a1 + a2 + a3
	cltd
	movl	-40(%ebp), %ecx         
								# %ecx = 5
	idivl	%ecx				# %ecx = %eax % %ecx = (a1 + a2 + a3) % 5
	cmpl	$0, %edx			# compare (a1 + a2 + a3) % 5 to 0
	jne	.LBB0_10				# if (a1 + a2 + a3) != 0 pass the next 4 cmds
# BB#9:                                 
	movl	-12(%ebp), %eax		
	addl	-16(%ebp), %eax
	addl	-20(%ebp), %eax		# %eax = a1 + a2 + a3
	movl	%eax, -28(%ebp)		# h = a1 + a2 + a3
.LBB0_10:                               
	jmp	.LBB0_11				# end of content in the third 'for'
.LBB0_11:                               
	movl	-20(%ebp), %eax		
	subl	$3, %eax
	movl	%eax, -20(%ebp)		# a3 -= 3
	jmp	.LBB0_5					# loop in the third 'for'
.LBB0_12:                              
	jmp	.LBB0_13				# end of the third 'for'
								# executed when break (line 66) or a3 < 0 in the third 'for' (line 58)
.LBB0_13:                               
	movl	-12(%ebp), %eax		
	subl	$2, %eax
	movl	%eax, -12(%ebp)		# a1 -= 2
	jmp	.LBB0_3					# loop in the second 'for'
.LBB0_14:                               
	jmp	.LBB0_15				# end of the second 'for'
								# executed when a1 < 0 in the second 'for' (line 39)
.LBB0_15:                             
	movl	-16(%ebp), %eax		
	addl	$-1, %eax
	movl	%eax, -16(%ebp)		# a2--
	jmp	.LBB0_1					# loop in the first 'for'
.LBB0_16:						# end of the first 'for'
	xorl	%eax, %eax			# clear eax
	addl	$36, %esp			# free the space for local variable
	popl	%esi				# restore %esi
	popl	%ebp				# restore %ebp
	retl						# restore context, return to caller
.Lfunc_end0:
	.size	main, .Lfunc_end0-main


	.ident	"clang version 3.8.0-2ubuntu4 (tags/RELEASE_380/final)"
	.section	".note.GNU-stack","",@progbits
```

改为`-m64`选项后，生成的汇编代码如下（`main`内主要计算过程类似，不做注释）

```assembly
	.text
	.file	"simple.c"
	.globl	main
	.align	16, 0x90
	.type	main,@function
main:                                  
	.cfi_startproc
# BB#0:
	pushq	%rbp
.Ltmp0:
	.cfi_def_cfa_offset 16
.Ltmp1:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp			# from %esp, %ebp to %rsp, %rbp: 64bit
.Ltmp2:
	.cfi_def_cfa_register %rbp	# no explicit allocation of stack space
	movl	$0, -4(%rbp)
	movl	$0, -24(%rbp)
	movl	$100, -20(%rbp)
	movl	-20(%rbp), %eax
	movl	%eax, -12(%rbp)
.LBB0_1:                               
                                      
                                       
	cmpl	$0, -12(%rbp)
	jl	.LBB0_16
# BB#2:                               
	movl	$2, %eax
	movl	-20(%rbp), %ecx
	movl	-20(%rbp), %edx
	addl	-12(%rbp), %edx
	movl	%eax, -28(%rbp)        
	movl	%edx, %eax
	cltd
	movl	-28(%rbp), %esi        
	idivl	%esi
	subl	%edx, %ecx
	movl	%ecx, -8(%rbp)
.LBB0_3:                             
                                        
                                     
	cmpl	$0, -8(%rbp)
	jl	.LBB0_14
# BB#4:                             
	movl	$3, %eax
	movl	-20(%rbp), %ecx
	movl	-20(%rbp), %edx
	addl	-12(%rbp), %edx
	movl	%eax, -32(%rbp)         
	movl	%edx, %eax
	cltd
	movl	-32(%rbp), %esi         
	idivl	%esi
	subl	%edx, %ecx
	movl	%ecx, -16(%rbp)
.LBB0_5:                               
                                       
                                      
	cmpl	$0, -16(%rbp)
	jl	.LBB0_12
# BB#6:                                
	movl	-8(%rbp), %eax
	addl	-12(%rbp), %eax
	addl	-16(%rbp), %eax
	cmpl	-24(%rbp), %eax
	jg	.LBB0_8
# BB#7:                                
	jmp	.LBB0_12
.LBB0_8:                                
	movl	$5, %eax
	movl	-8(%rbp), %ecx
	addl	-12(%rbp), %ecx
	addl	-16(%rbp), %ecx
	movl	%eax, -36(%rbp)        
	movl	%ecx, %eax
	cltd
	movl	-36(%rbp), %ecx        
	idivl	%ecx
	cmpl	$0, %edx
	jne	.LBB0_10
# BB#9:                                
	movl	-8(%rbp), %eax
	addl	-12(%rbp), %eax
	addl	-16(%rbp), %eax
	movl	%eax, -24(%rbp)
.LBB0_10:                              
	jmp	.LBB0_11
.LBB0_11:                              
	movl	-16(%rbp), %eax
	subl	$3, %eax
	movl	%eax, -16(%rbp)
	jmp	.LBB0_5
.LBB0_12:                              
	jmp	.LBB0_13
.LBB0_13:                               
	movl	-8(%rbp), %eax
	subl	$2, %eax
	movl	%eax, -8(%rbp)
	jmp	.LBB0_3
.LBB0_14:                               
	jmp	.LBB0_15
.LBB0_15:                               
	movl	-12(%rbp), %eax
	addl	$-1, %eax
	movl	%eax, -12(%rbp)
	jmp	.LBB0_1
.LBB0_16:
	xorl	%eax, %eax
	popq	%rbp
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc


	.ident	"clang version 3.8.0-2ubuntu4 (tags/RELEASE_380/final)"
	.section	".note.GNU-stack","",@progbits
```

主要区别在于`%esp` `%ebp`寄存器变为了64位的`%rsp` `%rbp`，以及没有显式的为局部变量分配空间的操作。

加上一段无意义的函数调用再生成汇编代码（`main`之后一样的代码在此不重复贴出）

```c
void func() {
    int i = 0;
    i++;
}
int main() {
    int a1, a2, a3, n, h = 0;
    n = 100;
    for (a2 = n; a2 >= 0; a2--) {
        for (a1 = n - (n + a2) % 2; a1 >= 0; a1 -= 2) {
            for (a3 = n - (n + a2) % 3; a3 >= 0; a3 -= 3) {
                func();
                if (a1 + a2 + a3 <= h) break;
                if ((a1 + a2 + a3) % 5 == 0) h = a1 + a2 + a3;
            }
        }
    }
    return 0;
}
```

```assembly
	.text
	.file	"simple.c"
	.globl	func
	.align	16, 0x90
	.type	func,@function
func:                                   # @func
	.cfi_startproc
# BB#0:
	pushq	%rbp
.Ltmp0:
	.cfi_def_cfa_offset 16
.Ltmp1:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
.Ltmp2:
	.cfi_def_cfa_register %rbp
	movl	$0, -4(%rbp)
	movl	-4(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -4(%rbp)
	popq	%rbp
	retq
.Lfunc_end0:
	.size	func, .Lfunc_end0-func
	.cfi_endproc

	.globl	main
	.align	16, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# BB#0:
	pushq	%rbp
.Ltmp3:
	.cfi_def_cfa_offset 16
.Ltmp4:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
.Ltmp5:
	.cfi_def_cfa_register %rbp
	subq	$48, %rsp					# space for local variables
	movl	$0, -4(%rbp)
	movl	$0, -24(%rbp)
	movl	$100, -20(%rbp)
	movl	-20(%rbp), %eax
	movl	%eax, -12(%rbp)
.LBB1_1:    
	....

```

可见`main`中产生了为局部变量分配空间的操作。故猜测原因为`main`已为进程栈中会出现的最顶端的函数，不会有子函数覆盖其局部变量

## 总结

汇编代码与源码的对应不是很难，而且可以观察到汇编代码中还有很多可以优化的部分（例如立即数可以直接存到要用的位置，减少一次赋值；同一个变量可以直接从寄存器赋给寄存器，不用两次读取内存）