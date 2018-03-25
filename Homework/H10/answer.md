环境：Ubuntu 16.04 LTS，gcc  5.4.0

1. 考虑如下的c代码

   ```c
   void func() {
       while(1);
   }
   int main() {
       long a = 1;
       long* p = &a;
       p[a] = &func;
   }
   ```

   将a设为何值时可以试程序陷入死循环？列出此时main的活动记录

   ​

   解答：将a设为4可将返回地址改为函数func的地址，使程序陷入func中的死循环

   test目录下的test.c文件即为源码，使用`make q1`进行编译，`make runq1`进行测试

2. 考虑如下两个c文件test1.c, test2.c

   ```c
   int buf[1] = {100};
   ```

   ```c
   #include <stdio.h>
   extern int *buf;
   int k[1] = {100};
   int *p = k;
   int main() {
       printf("%d, %d\n", k, p);
       printf("%d\n", buf);
   }
   ```

   编译方式:

   ```shell
   $gcc test1.c test2.c
   ```

   执行可执行文件的结果是什么？为什么会这样？

   ​

   解答：第一行printf输出了{100}在全局数据段中的地址，第二行输出了值100.

   各个变量在汇编文件中的表示形式都是label：

   ```assembly
   buf:
   	.long	100
   k:
   	.long	100
   p:
   	.quad	k
   ```

   buf、k、p的类型在汇编文件中是没有区别的，需要由编译器来记录它们在源码中的类型。

   在linux下AT&T格式的汇编，加上$符号代表立即寻址，不加则是代表直接寻址。在printf传参过程中，三个变量传入的方式分别为：

   ```assembly
   	movq	buf(%rip), %rax
   	movl	$k, %esi
   	movq	p(%rip), %rax
   ```

   从而，虽然k在c源码中可以被当作int*类型使用，但其实现方式是取其在数据段中的偏移作为立即数，而非像正常的指针变量一样取值，即：数组名在机器码中的角色与数组元素变量类似，只是由于编译器的区别对待使得数组名在源码中表现的与指针类似。

   test目录下的test1.c，test2.c文件即为源码，使用`make q2`进行编译，`make runq2`进行测试