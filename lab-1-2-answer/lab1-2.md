# Antlr的左递归处理

宋小牛 PB15000301

## 主要内容

* 分析和总结 ANTLR 对左递归文法的处理方法

## Antlr支持的左递归

左递归类型：

* 直接左递归

  ```
  A -> Aa | b
  ```
  这是 antlr v4 所支持的，前面的表达式就是一个例子。

  若将上述规则改为

  ```
  b -> b'a'
  ```

  即去掉规则中非左递归的部分，编译将报错

  ```shell
  error(147): UnsupportedLeftRecursive.g4:3:0: left recursive rule b must contain an alternative which is not left recursive
  ```

  从而 antlr v4 支持的直接左递归必须有非左递归的可选项

* 间接左递归

  ```
  S -> Aa | b
  A -> Sc | d
  ```
  这是不被 antlr v4 支持的。构造测试文法`UnsupportedLeftRecursive.g4`

  ```
  grammar UnsupportedLeftRecursive;
  s:
      a '+' a 
      | '@'
  ;

  a:
      s '*' s 
      | '@'
  ;
  ```

  尝试编译，报错如下

  ```shell
  $ antlr4 UnsupportedLeftRecursive.g4 
  error(119): UnsupportedLeftRecursive.g4::: The following sets of rules are mutually left-recursive [s, a]
  error(99): UnsupportedLeftRecursive.g4::: grammar UnsupportedLeftRecursive has no rules
  ```

  即不支持间接左递归

* 隐藏左递归

  ```
  A -> BA
  B -> epsilon
  ```
  这也是不被 antlr v4 支持的。修改测试文法`UnsupportedLeftRecursive.g4`

  ```
  grammar UnsupportedLeftRecursive;
  b:
      c b | '*'
  ;

  c:
      '*'?
  ;
  ```

  尝试编译，报错如下

  ```shell
  $ antlr4 UnsupportedLeftRecursive.g4 
  error(119): UnsupportedLeftRecursive.g4::: The following sets of rules are mutually left-recursive [b]
  error(99): UnsupportedLeftRecursive.g4::: grammar UnsupportedLeftRecursive has no rules
  ```

  即不支持隐藏左递归


综上，antlr v4 能识别出文法中的各类型左递归并且对其支持的左递归——直接左递归——进行处理。至于为何不支持非直接左递归，猜测原因为：直接左递归覆盖了大部分情况，并且处理非直接左递归耗时(消耗过多资源)，以及生成的语法规则顺序与原规则相去甚远。

## MultFirst and PlusFirst

编写新的文法`MultFirst.g4`，`PlusFirst.g4`，其区别为`exp`中`Multiply`和`Plus`的先后顺序不一样。编写`MultFirst.py`与`PlusFirst.py`来调用这两个文法生成的分析器，对输入的表达式进行加括号处理。

* 依赖环境：antlr4-python3-runtime-4.7，[下载链接](https://pypi.python.org/pypi/antlr4-python3-runtime/)，解压并在其目录执行

  ```shell
  python3 setup.py build
  python3 setup.py install
  ```


* MultFirst中， * 算符优先级大过 + 

  ```shell
  $ antlr4 MultFirst.g4 -Dlanguage=Python3
  $ python3 MultFirst.py 
  Please enter a expression: a + 1 * 0x32+(b+c)*d 
  ((a + (1 * 0x32)) + (((b + c)) * d))
  ```

* PlusFirst中，+ 算符优先级大过 * 

  ```shell
  $ antlr4 PlusFirst.g4 -Dlanguage=Python3
  $ python3 PlusFirst.py 
  Please enter a expression: a + 1 * 0x32+(b+c)*d 
  (((a + 1) * (0x32 + ((b + c)))) * d)
  ```

从而对于多个可选的左递归项，其优先级从上到下降低：这与 antlr 消除左递归的方法有关。根据[Adaptive LL(*) Parsing](https://pdfs.semanticscholar.org/e21c/6ac4b9332d599547b7670c33ce8d6bdc1966.pdf) 中的附录C，antlr会将直接左递归重写为多条非贪婪的无左递归的规则，其顺序(优先级)与原顺序一致，并基于 Adaptive LL(*) 来生成分析器