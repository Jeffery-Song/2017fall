#lab1-3

宋小牛 PB15000301

## 主要任务

修改、完善`src/syntax_tree_builder.cpp`，并以`compilationUnit`作为开始符号来解析.c1文件，生成AST。

## 重点难点

* 了解、使用ANTLR分析树的编程接口，书面总结它们与你在lab1-2中写的文法之间的关系。
* 学习使用Vistor访问者模式来构建AST
* 深入理解ANTLR的分析原理，结合生成的解析器源码以及调用的ANTLR运行时的实现代码，调研相关的文献，回答：
  * ATN的英文全称是什么，它代表什么？
  * SLL的英文全称是什么，它和LL、以及ALL(*)的区别是什么？它们分别怎么对待文法二义、试探回溯？
  * 了解并总结enterRecursionRule、unrollRecursionContexts、adaptivePredict函数的作用、接口和主要处理流程
  * 错误处理机制

## 修改`src/syntax_tree_builder.cpp`

这其中大部分的visit*函数都是一个套路，大体思路都是

1. 建好对应的*_syntax变量，填好line & pos
2. 根据终结符或非终结符数量来判断产生式
3. 根据产生式来visit各非终结符子节点，填好result的成员

这其中值得注意的有：

* 声明数组时不显式填写数组长度的情况，应当根据左右中括号的位置来判断，而非根据exp和comma的数量来判断，否则将与指出了数组长度，但未完整初始化的情况弄混

* block_syntax中的body是指向stmt的ptr_list，而block中允许decl语句，decl若使用assemble结构体来存放各个def，从而各def已被cast为global_def_syntax，而这不是由stmt_syntax继承而来的，强制转换为var_def_stmt_syntax再cast为stmt_syntax将无法正常运行。

  解决方案：在constdecl/vardecl中不建立assembly变量，而是直接建立一个ptr_list\<var_def_stmt_syntax\>并返回它。

  ```c++
  antlrcpp::Any syntax_tree_builder::visitVardecl(C1Parser::VardeclContext *ctx)
  {
      auto defs = ctx->vardef();
      auto def_list = new ptr_list<var_def_stmt_syntax>;
      for (auto def : defs) {
          ptr<var_def_stmt_syntax> def_;
          def_.reset(visit(def).as<var_def_stmt_syntax *>());
          def_list->push_back(def_);
      }
      return static_cast<ptr_list<var_def_stmt_syntax> *>(def_list);
  }
  ```

  ​

* block中有(decl|stmt)*，而各个def与stmt应按照其原文顺序存于block中(compilationUnit中也有类似的情况)

  解决方案：

  * 根据getLine与getCharPositionInLint返回值来判断先后顺序

  * 阅读ctx->exp()源码

    ```c++
    std::vector<C1Parser::ExpContext *> C1Parser::ConstdefContext::exp() {
      return getRuleContexts<C1Parser::ExpContext>();
    }

    namespace antlr4 {
      class ANTLR4CPP_PUBLIC ParserRuleContext : public RuleContext {
        template<typename T>
        std::vector<T *> getRuleContexts() {
          std::vector<T *> contexts;
          for (auto child : children) {
            if (antlrcpp::is<T *>(child)) {
              contexts.push_back(dynamic_cast<T *>(child));
            }
          }
          return contexts;
        }
      }
    }
    ```

    可见获取所有exp()子节点实际上是遍历children成员并取出所有指定类型的成员，从而可以直接遍历children，逐个判断其是decl还是stmt。

  最终采用了第二种解决方案，block中的实现如下，compilationUnit类似

  ```c++
  antlrcpp::Any syntax_tree_builder::visitBlock(C1Parser::BlockContext *ctx)
  {
      auto result = new block_syntax;
      result->line = ctx->getStart()->getLine();
      result->pos = ctx->getStart()->getCharPositionInLine();
      for (auto child : ctx->children) {
          if (antlrcpp::is<C1Parser::StmtContext *>(child)) {
              ptr<stmt_syntax> ptr1;
              ptr1.reset(visit(child).as<stmt_syntax *>());
              result->body.push_back(ptr1);
          }
          if (antlrcpp::is<C1Parser::DeclContext *>(child)) {
              auto ret = visit(child);
              for (auto def : *(ptr_list<var_def_stmt_syntax> *)ret) {
                  result->body.push_back(def);
              }
          }
      }
      return static_cast<block_syntax *>(result);
  }
  ```
### 测试 

构造一个基本用到所有产生式的样例 

```c
void func() {
    int a = 3;
    b = 4;
    c[7] = 9;
}
int v1, v2 = 3;
int a1[3];
int a2[3] = {1, 2};
int a3[] = {1, 2, 3, 4};
const int c1 = 3, a4[2] = {1, 2};

void main()
{
    int v1, v2 = 3;
    int a1[3];
    int a2[3] = {1, 2};
    int a3[] = {1, 2, 3, 4};
    const c1 = 3, a4[] = {1, 2};
    if(i > 0) i = 1;
    else i = -1;
    i = (i + 1 - 1) * 3 / 2 % 3;
    while (i > 0) i = i - 1;
    if(i == 0) i = 1;
    func();
    ;;
}
```

运行

```shell
jeffery@jeffery-N551JM:~/CP/c1recognizer-zhujiao/build$ ./c1r_test ../test/test_cases/my_test.c1 >> result
Warning at position 18:10 Type missing here. Assuming 'Int'
```

生成的AST见`doc/result`

## ANTLR分析原理

### 文法分析

$ATN$是指Augmented transition networks，是由输入的文法生成的与原文法等价的一个图（状态机），有一个开始节点和终止结点，并且允许递归。文法中每个非终结符都有自己的ATN。$ANTLR$的文法分析均基于$ATN$

SLL定义：[llkparser](http://www.slkpg.com/llkparse.html)

> The strong $LL(k)$ grammars are a subset of the $LL(k)$ grammars that can be parsed without knowledge of the left-context of the parse. That is, each parsing decision is based only on the next $k$ tokens of the input for the current nonterminal that is being expanded. A definition of the strong $LL(k)$ grammars follows.
>
> Definition: A grammar $G = ( N, T, P, S )$ is said to be strong $LL(k)$ for some fixed natural number $k$ if for all nonterminals $A$, and for any two distinct $A$-productions in the grammar
> ​        $A$ --> $\alpha$
> ​        $A$ --> $\beta$
>
> $FIRST_k ( \alpha FOLLOW_k (A) ) \cap FIRST_k ( \beta FOLLOW_k (A) ) = Ø$

LL(k)定义：[LL parser](https://en.wikipedia.org/wiki/LL_parser)

> An $LL$ parser is called an $LL(k)$ parser if it uses $k$ [tokens](https://en.wikipedia.org/wiki/Token_(parser)) of [lookahead](https://en.wikipedia.org/wiki/Parsing#Lookahead) when parsing a sentence. If such a parser exists for a certain grammar and it can parse sentences of this grammar without [backtracking](https://en.wikipedia.org/wiki/Backtracking) then it is called an $LL(k)$ grammar.
>
> ...
>
> Let $G$ be a context-free grammar and $k\geq 1$. We say that $G$ is $LL(k)$, if and only if for any two leftmost derivations:
>
> 1. $S\Rightarrow...\Rightarrow\omega A\alpha\Rightarrow...\Rightarrow\omega\beta\alpha\Rightarrow ... \Rightarrow\omega x$
> 2. $S\Rightarrow...\Rightarrow\omega A\alpha\Rightarrow...\Rightarrow\omega\gamma\alpha\Rightarrow ... \Rightarrow\omega y$
>
> Following conditions holds: Prefix of the string $x$ of length $k$ equals the prefix of the $y$ of length $k$ implies $\beta = \gamma$

$LL(k)$意思就是说，根据接下来$k$个符号($x,y$的前缀)可以唯一决定使用哪一条产生式(或称接下来的动作)($A\Rightarrow\beta$)

综合来说，$LL(k)$是不需要回溯的，其根据已经使用的产生式以及读入的$k$个符号来判断使用哪一个产生式。而$SLL(k)$不需要其上文，只需要根据接下来的$k$个符号即可判断使用哪一个产生式，$SLL(k)$是$LL(k)$的一个子集，从而$SLL(k)$也不需要回溯。$SLL(k)、LL(k)$均不允许二义文法

这里有一个$LL(2)$但不是$SLL(2)$$的例子

>$S$ --> $aAaa$
>
>$S$ --> $bAba$
>
>$A$ --> $b$
>
>$A$ --> $\epsilon$

该文法必是$LL(2)$的，因不存在上文提到的冲突

1. $S\Rightarrow aAaa\Rightarrow abaa$
2. $S\Rightarrow bAba\Rightarrow bbba$
3. $S\Rightarrow aAaa\Rightarrow aaa$
4. $S\Rightarrow bAba\Rightarrow bba$

其中1、3里$x=baa,y=aa$，2、4里$x=bba, y=ba$，前缀不同

然而$FOLLOW_2(A)={aa,ba}$，$FIRST_2(\{baa,bba\}) \cap FIRST_2(\{aa,ba\})\neq Ø$，从而该文法不是$SLL(2)$的。

可以看出，该文法不是$SLL(2)$主要是因为忽略了读入的$w$(即$A$之前的$a、b$)的信息。

[$ALL(k)$](https://pdfs.semanticscholar.org/e21c/6ac4b9332d599547b7670c33ce8d6bdc1966.pdf)，即$Adaptive\ LL(k)$，是一种$LL(k)$的分析方法，它综合了自上而下分析的简洁以及$GLR$的决策能力。具体而言，$LL(k)$采用的是静态分析方法，它在每个决策点暂停，并等预测机制选择了合适的产生式后继续运行。一种重要的优化方法是将语法分析移到运行时，这样静态的语法分析就不再需要了。静态分析需要考虑所有可能的输入序列，而动态分析只需要考虑有限的已看到的输入序列。

$ALL$采用的方法是在每个不确定的决策点启动多个子分析器，每个分析器对应一种可能。它们以伪并行的方式遍历所有的可能，当某个子分析器匹配失败的时候即被杀掉，直到最后只剩一个匹配成功的子分析器。如果直到文件结尾还有多个存活的子分析器，则分析器会优先选择使用了编号较小的产生式的那个分析器。从而，$ALL$没有回溯，但是允许一定程度上的二义。

$ANTLR$实际上采用的是两阶段分析：先使用$SLL$尝试分析，这时候如果出错，可能是$SLL$不够强，也可能是语法错误，这时候再使用$ALL$来分析。论文中提到实际情况中，$SLL$涵盖了大部分情况，所以这种方法是有实际意义的。

### 部分接口

`enterRecursionRule`：该接口只在出现了左递归的产生式中出现过

```c++
C1Parser::CondContext* C1Parser::cond() {
  CondContext *_localctx = _tracker.createInstance<CondContext>(_ctx, getState());
  enterRule(_localctx, 20, C1Parser::RuleCond);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  ...
}

C1Parser::ExpContext* C1Parser::exp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  C1Parser::ExpContext *_localctx = _tracker.createInstance<ExpContext>(_ctx, parentState);
  C1Parser::ExpContext *previousContext = _localctx;
  size_t startState = 22;
  enterRecursionRule(_localctx, 22, C1Parser::RuleExp, precedence);

    size_t _la = 0;

  auto onExit = finally([=] {
    unrollRecursionContexts(parentContext);
  });
  ...
}
```

从而enterRecursionRule是用来处理左递归文法的，unrollRecursionRule则是用来退出左递归文法

对比enterRule与enterRecursionRule

```c++
void Parser::enterRule(ParserRuleContext *localctx, size_t state, size_t /*ruleIndex*/) {
  setState(state);
  _ctx = localctx;
  _ctx->start = _input->LT(1);
  if (_buildParseTrees) {
    addContextToParseTree();
  }
  if (_parseListeners.size() > 0) {
    triggerEnterRuleEvent();
  }
}

void Parser::enterRecursionRule(ParserRuleContext *localctx, size_t state, size_t /*ruleIndex*/, int precedence) {
  setState(state);
  _precedenceStack.push_back(precedence);
  _ctx = localctx;
  _ctx->start = _input->LT(1);
  if (!_parseListeners.empty()) {
    triggerEnterRuleEvent(); // simulates rule entry for left-recursive rules
  }
}
```

可以看到exp的enterrecursionRule多了一个优先级参数。根据$ANTLR$官网配书，这是消除左递归时新增的参数，以此来判断是否可用某条产生式。该函数比没有左递归的版本多了一步优先级入栈，即类似函数调用的传参操作。

```c++
void Parser::exitRule() {
  if (_matchedEOF) {
    // if we have matched EOF, it cannot consume past EOF so we use LT(1) here
    _ctx->stop = _input->LT(1); // LT(1) will be end of file
  } else {
    _ctx->stop = _input->LT(-1); // stop node is what we just matched
  }

  // trigger event on ctx, before it reverts to parent
  if (_parseListeners.size() > 0) {
    triggerExitRuleEvent();
  }
  setState(_ctx->invokingState);
  _ctx = dynamic_cast<ParserRuleContext *>(_ctx->parent);
}

void Parser::unrollRecursionContexts(ParserRuleContext *parentctx) {
  _precedenceStack.pop_back();
  _ctx->stop = _input->LT(-1);
  ParserRuleContext *retctx = _ctx; // save current ctx (return value)

  // unroll so ctx is as it was before call to recursive method
  if (_parseListeners.size() > 0) {
    while (_ctx != parentctx) {
      triggerExitRuleEvent();
      _ctx = dynamic_cast<ParserRuleContext *>(_ctx->parent);
    }
  } else {
    _ctx = parentctx;
  }

  // hook into tree
  retctx->parent = parentctx;

  if (_buildParseTrees && parentctx != nullptr) {
    // add return ctx into invoking rule's tree
    parentctx->addChild(retctx);
  }
}
```

左递归版本的多了对优先级的栈操作，以及\_ctx的unroll。enterRecursionRule中缺少的\_buildParseTrees被移到了该处

adaptivePredic则是用来判断使用哪条可选择的以非终结符开头的产生式，根据其返回值来switch

在每个文法符号的类方法中，都有一个名称与其文法名字相同的文法，例如exp：

```c++
C1Parser::ExpContext* C1Parser::exp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  C1Parser::ExpContext *_localctx = _tracker.createInstance<ExpContext>(_ctx, parentState);
  C1Parser::ExpContext *previousContext = _localctx;
  size_t startState = 22;
  enterRecursionRule(_localctx, 22, C1Parser::RuleExp, precedence);

    size_t _la = 0;

  auto onExit = finally([=] {
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(178);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case C1Parser::Plus:
      case C1Parser::Minus: {
        setState(170);
        _la = _input->LA(1);
        if (!(_la == C1Parser::Plus

        || _la == C1Parser::Minus)) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(171);
        exp(4);
        break;
      }

      case C1Parser::LeftParen: {
        setState(172);
        match(C1Parser::LeftParen);
        setState(173);
        exp(0);
        setState(174);
        match(C1Parser::RightParen);
        break;
      }

      case C1Parser::Identifier: {
        setState(176);
        lval();
        break;
      }

      case C1Parser::Number: {
        setState(177);
        match(C1Parser::Number);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(188);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 19, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(186);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 18, _ctx)) {
        case 1: {
          _localctx = _tracker.createInstance<ExpContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExp);
          setState(180);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(181);
          _la = _input->LA(1);
          if (!((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & ((1ULL << C1Parser::Multiply)
            | (1ULL << C1Parser::Divide)
            | (1ULL << C1Parser::Modulo))) != 0))) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(182);
          exp(7);
          break;
        }

        case 2: {
          _localctx = _tracker.createInstance<ExpContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExp);
          setState(183);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(184);
          _la = _input->LA(1);
          if (!(_la == C1Parser::Plus

          || _la == C1Parser::Minus)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(185);
          exp(6);
          break;
        }

        } 
      }
      setState(190);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 19, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}
```

其中每个switch的分支即对应一条产生式，每匹配一个符号之前要先setstate，再调用对应的文法符号的函数，若是含左递归的文法符号则还需加上其优先级