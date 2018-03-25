# H2

宋小牛 PB15000301

[TOC]

## 代码结构

主代码为`main.cc`

转换表如下

| state | $\varepsilon$ |       a       |       b       |
| :---: | :-----------: | :-----------: | :-----------: |
|   0   |      1,3      | $\varnothing$ | $\varnothing$ |
|   1   | $\varnothing$ |       2       | $\varnothing$ |
|   2   | $\varnothing$ |       2       | $\varnothing$ |
|   3   | $\varnothing$ | $\varnothing$ |       4       |
|   4   | $\varnothing$ | $\varnothing$ |       4       |

### 转换表初始化

```c++
std::vector<std::unordered_map<char, std::vector<int>>> table = {
  { {255, {1, 3}}, {'a', {}}, {'b', {}} },
  { {255, {}}, {'a', {2}}, {'b', {}} },
  { {255, {}}, {'a', {2}}, {'b', {}} },
  { {255, {}}, {'a', {}}, {'b', {4}} },
  { {255, {}}, {'a', {}}, {'b', {4}} }
};
```

使用ASCII码255代表 $\varepsilon$

### 结束态集合

```c++
std::set<int> endstate = {2, 4};
```

### 判断函数

```c++
//从 string s 的偏移 c，状态 state 开始判断是否满足正规式，返回值非 0 表示识别出的串结尾，0表示不符合
int func(int c, int state) {
  int end = 0;
  if (c == s.size()) {								//若已到字符串结尾
    if (endstate.find(state) != endstate.end()) {	//若状态可接受，当前偏移即识别出的串结尾
      return c;
    } else {
      return 0;										//0 表示识别失败
    }
  }
  if (endstate.find(state) != endstate.end() ) {	//状态可接受，当前偏移即识别出的串结尾
    end = c;
  }
  for (int it : table[state][255]) {				//走 epsilon 边
    int ret = func(c, it);							//递归调用，识别出更长的则更新结尾
    if (ret > end) end = ret;
  }
  for (int it : table[state][s[c]]) {				//走非 epsilon 边
    int ret = func(c + 1, it);						//递归调用，识别出更长的则更新结尾
    if (ret > end) end = ret;
  }
  return end;
}
```

### 主函数

```c++
int main() {
  getline(std::cin, s);						//s 为全局变量，读取一整行
  int begin = 0;							//首先从第一个字符开始判断
  while (begin < s.size()) {
    int ret = func(begin, 0);
    if (ret) {								//若返回值非 0，识别出串，ret即为结尾
      std::cout << "match found:";
      for (int i = begin; i < ret; i++) {
        std::cout << s[i];					//输出串
      }
      std::cout << std::endl;
      begin = ret;							//从识别出的串结尾开始识别
    } else {
      begin++;								//识别失败，从下一个字符开始
    }
  }
  return 0;
}
```

## Usage

1. 初始化自己需要的转换表与结束状态集
2. 编译

```shell
clang++ main.cc -std=c++11
```

3. 运行

```shell
./a.out
aaabbababdkh bdbaaa
match found:aaa
match found:bb
match found:a
match found:b
match found:a
match found:b
match found:b
match found:b
match found:aaa
```

## `aaab`的识别过程

在`func`中加入递归层数以及输出当前状态的语句，修改后的文件为`main-debug.cc`

```c++
int func(int c, int state, int depth) {
  for (int i = 0; i < depth; i++) {
    std::cout << "    ";
  }
  std::cout << "begin:" << c << " state:" << state << std::endl;
  int end = 0;
  if (c == s.size()) {
    for (int i = 0; i < depth; i++) {
      std::cout << "    ";
    }
    if (endstate.find(state) != endstate.end()) {
      std::cout << "end:" << c << std::endl;
      return c;
    } else {
      std::cout << "end:" << 0 << std::endl;
      return 0;
    }
  }
  if (endstate.find(state) != endstate.end() ) {
    end = c;
  }
  for (int it : table[state][255]) {
    int ret = func(c, it, depth + 1);
    if (ret > end) end = ret;
  }
  for (int it : table[state][s[c]]) {
    int ret = func(c + 1, it, depth + 1);
    if (ret > end) end = ret;
  }
  for (int i = 0; i < depth; i++) {
    std::cout << "    ";
  }
  std::cout << "end:" << end << std::endl;
  return end;
}
```

`main`中调用`func`的位置加上深度0，识别`aaab`的输出如下

```shell
jeffery@jeffery-N551JM:~/Codes/MyCode/CP/aa*$ ./a.out 
aaab
begin:0 state:0
    begin:0 state:1				#经 epsilon 到 state 1
        begin:1 state:2				#经 a 到 state 2
            begin:2 state:2				#经 a 到 state 2
                begin:3 state:2				#经 a 到 state 2
                end:3						#当前态是接受态，b 无边可走，end = 3
            end:3
        end:3
    end:3
    begin:0 state:3				#经 epsilon 到 state 3
    end:0						#当前态非接受态，a 无边可走，end = 0
end:3
aaa							#识别出串 aaa
begin:3 state:0				#从 aaa 的结尾开始识别
    begin:3 state:1				#经 epsilon 到 state 1
    end:0						#非接受，b 无边可走，end = 0
    begin:3 state:3				#经 epsilon 到 state 3
        begin:4 state:4				#经 b 到 state 4
        end:4						#接受态，已达 s 结尾，end = 4
    end:4
end:4
b							#识别出串 b

```

