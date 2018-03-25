# CSA

宋小牛 PB15000301

##3.1

生成方法：根据文档中指令生成.dot文件，再用dot指令转为.svg文件

test.c:程序源码

AST:抽象语法树，每个语法单位一个结点

CFG:遍历抽象语法树生成的控制流图，每个BasicBlock一个结点，其内容为中间代码

ExplodedGraph:基于CFG生成的运行时轨迹，每个结点代表了运行中某个时间点的状态，内容包括程序执行到的位置，当前执行代码的symbolic value的内容、位置与性质

##3.2

1. Checker对程序的分析主要在CFG上进行
2. Checker分析程序时记录的程序状态保存在ProgramState中的GenericDataMap
3. 编写了3.2.c并生成ExplodedGraph为3.2.svg
   * 栈中x的实体、栈中y的实体
   * 引用变量x时创建的新SVal、对该新Sval取地址时创建的新SVal、栈中p的实体
   * 引用变量p时创建的新SVal、对该新Sval取值时创建的新Sval、为常数1创建的SVal、储存p+1的SVal、对p+1取值创建的SVal。

##3.3

1. 智能指针包括shared_ptr、unique_ptr、weak_ptr

   * unique_ptr:独享所有权的只能指针，无法使两个unique_ptr指向同一个对象，但可以进行移动构造和移动赋值操作，从而可在容器中保存指针


   * shared_ptr:同一资源可被多个指针共享，使用计数机制来表明资源被几个指针共享。指针被析构时计数减1，计数变为0时资源被释放。可能出现两个shared_ptr相互引用时导致死锁的情况


   * weak_ptr:用于解决shared_ptr死锁问题，不增加对象的引用计数，与shared_ptr可以相互转化

2. c++ RTTI缺点之一：dynamic_cast<>仅对有虚函数表的类有效。llvm提供了LLVM-style RTTI来代替c++ RTTI，提供了5个模板:

   * isa<>: 判断是否是指定类的实例
   * cast<>: 已做类型检查后做类型转换，类型错误则产生failure
   * dyn_cast<>: 未做类型检查即直接做转换，类型错误则返回nullptr
   * cast_or_null<>: 允许传入null
   * dyn_cast_or_null<>: 允许传入null

3. 应声明参数为std::vector，因vector有参数为数组的构造函数。类似的，对字符串应声明为std::string

4. 匿名空间显式的向编译器指明当前namespace的内容尽在这一编译单元可见，从而允许编译器做更加激进的优化以及消除部分可能的符号名称冲突

##3.4

1. checker对file指针保存了其是否指向一个打开的文件这一状态，该状态保存于ExplodedGraph中的ProgramState中

2. 当调用了fopen或fclose时，checkPostCall(checkPreCall)会被调用，将在streamMap中set一个对应的状态

3. 在set指针为close之前会先判断是否已为close

   判断是否泄露时要判断指针是否为open

4. 若当前操作保证不会将file指针close，直接返回。否则从state中删除invalidaedSymbols

   消亡的symbol不再对其进行追踪

   在离开一个作用域时调用

5. 多次close

   ```c
   #include<stdio.h>
   int foo() {
       FILE* p;
       p = fopen("t.txt", "r");
       fclose(p);
       fclose(p);
       return 0;
   }
   ```

   ```shell
   $ ../build/bin/clang --analyze -Xanalyzer -analyzer-checker=alpha.unix.SimpleStream 3.4.c
   3.4.c:6:5: warning: Closing a previously closed file stream
       fclose(p);
       ^~~~~~~~~
   1 warning generated.
   ```

   部分的泄露

   ```c
   #include<stdio.h>
   int foo() {
       FILE* p;
       p = fopen("t.txt", "r");
   }
   ```

   ```shell
   $ ../build/bin/clang --analyze -Xanalyzer -analyzer-checker=alpha.unix.SimpleStream 3.4.c
   3.4.c:5:1: warning: Opened file is never closed; potential resource leak
   }
   ^
   1 warning generated.
   ```

   当从局部作用域获取file指针，局部的指针无法被继续追踪

   ```c
   #include <stdio.h>
   int foo() {
       FILE* p;
       int i = 3;
       while (i * i < 36945616) {
           FILE* q = fopen("t", "r");
           p = q;
           fclose(q);
           i++;
       }
       fgetc(p);
   }
   ```

   无warning

   已被关闭的指针再尝试读取时无warning

   ```c
   #include <stdio.h>
   int foo() {
       FILE* p;
       p = fopen("t.txt", "r");
       fgetc(p);
       fclose(p);
       fgetc(p);
   }
   ```

##3.5

1. 需要增加一个checker的实现文件，并在实现中增加register语句

   在checkers.td中对应package中添加描述

   在CMakeLists.txt中添加该实现文件

2. 这是在`tools/clang/cmake/modules/AddClang.cmake`中定义的cmake函数

   其功能是利用tablegen，从Checkers.td生成clang所需的include文件：`<build-dir>/tools/clang/include/clang/StaticAnalyzer/Checkers/checkers.inc`

3. .td文件包含了tablegen代码，使用类似c++继承机制，生成.inc文件，这一文件可以方便的转换为其他语言的头文件，从而提高可移植性，减少代码量

