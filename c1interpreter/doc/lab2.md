# lab2 c1interpreter

宋小牛 PB15000301

[TOC]

##notes

node.accept(*this)来调用node的visit

全局变量无初值使用0来初始化

全局变量初值为常量表达式，局部变量无此限制

数组长度为常量表达式

constant_expected为true的情况：

* 全局变量的初始化
* 数组类型声明的length

lval_as_rval为false的情况：

* assign语句的target，注意target中的index应将lval_as_rval置true

in_global为flase的情况：

* visit函数内部body时

需要通过value_result传值的visit函数，当出错时需要置value_result为nullptr，在父节点中判断其是否为空，否则在使用value_result的父节点会将之前的value_result错误的当成子节点的结果

计算常量表达式时可能出现除以0的情况，应对此进行检查。而且由于无法判断是否因出现了变量使得const_result中的结果无效，故只能使用此方式避免因除以0试编译过程非正常结束。



## 构建assembly builder

首先是最基本的三个node

assembly

```c++
void assembly_builder::visit(assembly &node)
{
    for (auto global_defs : node.global_defs) {
        global_defs->accept(*this);
    }
}
```

func_def_syntax

```c++
void assembly_builder::visit(func_def_syntax &node)
{
    //检查是否重定义
    if (functions.find(node.name) != functions.end()) {
        err.error(node.line, node.pos, "conflict function declaration");
        error_flag = true;
        return;
    }
    //创建函数变量
    auto func = Function::Create(FunctionType::get(Type::getVoidTy(context), std::vector<Type *>(), false),
                                 GlobalValue::LinkageTypes::ExternalLinkage,
                                 node.name.data(), module.get());
    functions[node.name] = func;
    current_function = func;
    //每个block都要创建一个新的作用域，从而应在block中enter scope
    //enter_scope();
    auto func_entry = BasicBlock::Create(context, "entry", func);
    builder.SetInsertPoint(func_entry);

    in_global = false;
    node.body->accept(*this);
    builder.CreateRetVoid();
    in_global = true;
    //exit_scope();
    builder.ClearInsertionPoint();
}
```

block

```c++
void assembly_builder::visit(block_syntax &node)
{
    enter_scope();
    for (auto stmt : node.body) {
        stmt->accept(*this);
    }
    exit_scope();
}
```

### 表达式

其中值得注意的有

* 计算常量表达式的时候，可能出现除以0的情况，对其应进行检查
* 计算非常量表达式的时候，可能子节点的value_result为空，对此需要进行检查
* id不能出现在常量表达式中，出错时应置value_result为空
* 取下标的时候要将lval_as_rval置true

```c++
//双目运算符
void assembly_builder::visit(binop_expr_syntax &node)
{
    if (constexpr_expected) {
        node.lhs->accept(*this);
        int l = const_result;
        node.rhs->accept(*this);
        int r = const_result;
        switch (node.op) {
            case binop::plus :
                const_result = l + r;
                break;
            case binop::minus :
                const_result = l - r;
                break;
            case binop::multiply :
                const_result = l * r;
                break;
            case binop::divide :
                if (r == 0) {
                    err.error(node.line, node.pos, "divided by zero");
                    error_flag = true;
                    return;
                }
                const_result = l / r;
                break;
            case binop::modulo :
                if (r == 0) {
                    err.error(node.line, node.pos, "divided by zero");
                    error_flag = true;
                    return;
                }
                const_result = l % r;
                break;
            default :
                err.error(node.line, node.pos, "invalid opcode");
                error_flag = true;
                return;
        }
    } else {
        node.lhs->accept(*this);
        if (!value_result) return;
        Value *left_opr = value_result;
        node.rhs->accept(*this);
        if (!value_result) return;
        Value *right_opr = value_result;
        switch (node.op) {
            case binop::plus : 
                value_result = builder.CreateNSWAdd(left_opr, right_opr);
                break;
            case binop::minus :
                value_result = builder.CreateNSWSub(left_opr, right_opr);
                break;
            case binop::multiply :
                value_result = builder.CreateNSWMul(left_opr, right_opr);
                break;
            case binop::divide :
                value_result = builder.CreateSDiv(left_opr, right_opr);
                break;
            case binop::modulo :
                value_result = builder.CreateSRem(left_opr, right_opr);
                break;
            default:
                err.error(node.line, node.pos, "invalid opcode");
                error_flag = true;
                value_result = nullptr;
                return;
        }
    }
}
//单目运算符
void assembly_builder::visit(unaryop_expr_syntax &node)
{
    if (constexpr_expected) {
        node.rhs->accept(*this);
        int r = const_result;
        if (node.op == unaryop::minus) {
            const_result = -const_result;
        }
        else if (node.op != unaryop::plus) {
            err.error(node.line, node.pos, "invalid opcode");
            error_flag = true;
        }
    } else {
        node.rhs->accept(*this);
        if (!value_result) return;
        Value* r = value_result;
        if (node.op == unaryop::minus) {
            value_result = builder.CreateNSWNeg(r);
        }
        else if (node.op != unaryop::plus) {
            err.error(node.line, node.pos, "invalid opcode");
            error_flag = true;
            value_result = nullptr;
        }
    }
}
//常量表达式
void assembly_builder::visit(literal_syntax &node)
{
    if (constexpr_expected) {
        const_result = node.number;
    } else {
        value_result = ConstantInt::get(context, APInt(32, node.number));
    }
}
//左值表达式
void assembly_builder::visit(lval_syntax &node)
{
    if (lval_as_rval) {
        //id作为右值
        if (constexpr_expected) {
            err.error(node.line, node.pos, "constant expression expected");
            error_flag = true;
            value_result = nullptr;
            return;
        }

        std::tuple<Value*, bool, bool> v = lookup_variable(node.name);
        
        if (std::get<0>(v) == nullptr){
            //找不到对应名字的变量
            err.error(node.line, node.pos, "undefined variable " + node.name);
            error_flag = true;
            value_result = nullptr;
            return;
        }
        if (node.array_index) {
            //表达式中有数组下标
            if (std::get<2>(v) == false) {
                //对非数组变量取下标，报错
                err.error(node.line, node.pos, node.name + " is not an array");
                error_flag = true;
                value_result = nullptr;
                return;
            }
            //获取下标
            node.array_index->accept(*this);
            Value* index[] = {ConstantInt::get(context, APInt(32, 0)), value_result};
            //根据对应下标取元素地址，再取值
            value_result = builder.CreateInBoundsGEP(std::get<0>(v), index);
            value_result = builder.CreateLoad(value_result);
        } else if (std::get<2>(v) == true) {
            //无数组下标，但变量为数组，报错
            err.error(node.line, node.pos, "variable " + node.name + " is an array");
            error_flag = true;
            value_result = nullptr;
            return;
        } else {
            //直接取值
            value_result = builder.CreateLoad(std::get<0>(v));
        }
    } else {
        //id作为左值，取地址即可
        std::tuple<Value*, bool, bool> v = lookup_variable(node.name);

        if (std::get<0>(v) == nullptr){
            //未定义的变量
            err.error(node.line, node.pos, "undefined variable " + node.name);
            error_flag = true;
            value_result = nullptr;
            return;
        }

        if (std::get<1>(v)) {
            //常量变量，不允许作为左值
            err.error(node.line, node.pos, "left value is constant here");
            error_flag = true;
            value_result = nullptr;
            return;
        }
        if (node.array_index) {
            //有下标
            if (std::get<2>(v) == false) {
                //非数组类型，报错
                err.error(node.line, node.pos, node.name + " is not an array");
                error_flag = true;
                value_result = nullptr;
                return;
            }
            //计算下标的时候，其中的变量要作为右值处理
            lval_as_rval = true;
            node.array_index->accept(*this);
            lval_as_rval = false;
            //Value* index = value_result;
            Value* index[] = {ConstantInt::get(context, APInt(32, 0)), value_result};
            value_result = builder.CreateInBoundsGEP(std::get<0>(v), index);
        } else {
            //无下标
            if (std::get<2>(v) == true) {
                //数组类型，报错
                err.error(node.line, node.pos, node.name + " is an array");
                error_flag = true;
                value_result = nullptr;
                return;
            }
            value_result = std::get<0>(v);
        }
    }
}
```

### 非控制流语句

其中值得注意的有

* value_result为空时直接返回
* 数组变量的长度必须为常量表达式
* 全局变量初始化值应为常量表达式，无初值初始化为0

```c++
//函数调用语句
void assembly_builder::visit(func_call_stmt_syntax &node)
{
    if (functions.find(node.name) == functions.end()) {
        err.error(node.line, node.pos, "no function named " + node.name);
        error_flag = true;
        return;
    }
    builder.CreateCall(functions[node.name], {});
}
//赋值语句
void assembly_builder::visit(assign_stmt_syntax &node)
{
    lval_as_rval = false;
    node.target->accept(*this);
    lval_as_rval = true;
    Value* l = value_result;
    if (value_result == nullptr) {
        return;
    }
    node.value->accept(*this);
    if (value_result == nullptr) {
        return;
    }
    builder.CreateStore(value_result, l);   
}
//变量声明语句
void assembly_builder::visit(var_def_stmt_syntax &node)
{
    if (in_global) {
        //全局变量，分是否常量，是否为数组
        //常量必有initializer，非常量可能没有initializer，没有的情况置0
        //从而判断有无初值即可，无需判断是否为常数
        if (node.array_length != nullptr) {
            //计算数组长度
            constexpr_expected = true;
            node.array_length->accept(*this);
            constexpr_expected = false;
            int size = const_result;
            if (size <= 0) {
                //数组长度为非正数
                err.error(node.line, node.pos, "declare an array with zero or negtive size");
                error_flag = true;
                return;
            }
            //初始化列表大于数组长度
            if (node.initializers.size() > size) {
                err.error(node.line, node.pos, "initializers' size exceeded array size");
                error_flag = true;
                return;
            }
            //初值无论是否显式指明都要处理
            int i = 0;
            //初值为常量表达式
            constexpr_expected = true;
            std::vector<Constant*> init_v;
            //将各个计算出的初值放入一个vector
            for (auto initializer : node.initializers) {
                initializer->accept(*this);
                init_v.push_back(ConstantInt::get(Type::getInt32Ty(module->getContext()), const_result));
                i++;
            }
            constexpr_expected = false;
            //向剩余位置填入初值0
            auto zero = ConstantInt::get(context, APInt(32, 0));
            for (; i < size; i++) {
                init_v.push_back(zero);
            }
            //用生成好的vector初始化一个constantarray变量
            Constant* init = ConstantArray::get(ArrayType::get(Type::getInt32Ty(module->getContext()), size), init_v);
            //用constantarray变量初始化全局变量
            GlobalVariable* v = new GlobalVariable(*module,
                                                    ArrayType::get(Type::getInt32Ty(module->getContext()), size),
                                                    node.is_constant,
                                                    GlobalValue::ExternalLinkage,
                                                    init,
                                                    node.name);
            if (!declare_variable(node.name, v, node.is_constant, true)) {
                //变量名冲突
                err.error(node.line, node.pos, "variable " + node.name + " has been defined");
                error_flag = true;
                return;
            }
        } else {
            //无数组长度，非数组变量
            GlobalVariable* v;
            if (node.initializers.size()) {
                //提供了初值，按常量表达式visit
                constexpr_expected = true;
                node.initializers[0]->accept(*this);
                constexpr_expected = false;
                v = new GlobalVariable(*module,
                                                        Type::getInt32Ty(module->getContext()),
                                                        node.is_constant,
                                                        GlobalValue::ExternalLinkage,
                                                        ConstantInt::get(context, APInt(32, const_result)),
                                                        node.name);

            } else {
                //无初值
                v = new GlobalVariable(*module,
                                        Type::getInt32Ty(module->getContext()),
                                        node.is_constant,
                                        GlobalValue::ExternalLinkage,
                                        ConstantInt::get(context, APInt(32, 0)),
                                        node.name);
            }
            //声明变量
            if (!declare_variable(node.name, v, node.is_constant, false)) {
                err.error(node.line, node.pos, "variable " + node.name + " has been defined");
                error_flag = true;
                return;
            }
        }
    } else {
        //非全局变量，初始化值不再限制于常量表达式
        bool is_array = node.array_length != nullptr;
        if (is_array) {
            //数组变量，按照常量表达式visit长度
            constexpr_expected = true;
            node.array_length->accept(*this);
            constexpr_expected = false;
            int v_l_ = const_result;
            if (v_l_ <= 0) {
                //非正长度
                err.error(node.line, node.pos, "declare an array with zero or negtive size");
                error_flag = true;
                return;
            }
            //分配空间
            Value* v = builder.CreateAlloca(ArrayType::get(Type::getInt32Ty(module->getContext()), v_l_), nullptr, node.name);
            //声明
            if (!declare_variable(node.name, v, node.is_constant, is_array)) {
                err.error(node.line, node.pos, "variable " + node.name + " has been defined");
                error_flag = true;
                return;
            }
            //初始化列表长于数组大小
            if (node.initializers.size() > v_l_) {
                err.error(node.line, node.pos, "initializers' size exceeded array size");
                error_flag = true;
                return;
            }
            if (node.initializers.size()) {
                //有初值
                int i = 0;
                for (auto initializer : node.initializers) {
                    Value* index[] = {ConstantInt::get(context, APInt(32, 0)), ConstantInt::get(context, APInt(32, i))};
                    Value* ptr = builder.CreateGEP(v, index);
                    initializer->accept(*this);
                    if (value_result == nullptr) {
                        return;
                    }
                    builder.CreateStore(value_result, ptr);
                    i++;
                }
                //初始化列表可能没有给满，补0
                Value* zero = ConstantInt::get(context, APInt(32, 0));
                while (i < v_l_) {
                    Value* index[] = {ConstantInt::get(context, APInt(32, 0)), ConstantInt::get(context, APInt(32, i))};
                    Value* ptr = builder.CreateInBoundsGEP(v, index);
                    builder.CreateStore(zero, ptr);
                    i++;
                }
            }
        } else {
            //非数组
            Value* v = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, node.name);
            if (!declare_variable(node.name, v, node.is_constant, false)) {
                err.error(node.line, node.pos, "variable " + node.name + " has been defined");
                error_flag = true;
                return;
            }
            if (node.initializers.size()) {
                node.initializers[0]->accept(*this);
                if (value_result == nullptr) {
                    return;
                }
                Value* i = value_result;
                builder.CreateStore(i, v);

            }

        }
    }
}
```

### 控制流相关

其中值得注意的有

* if语句可能没有else块，应进行特殊处理
* while语句应为cond创建一个单独的basicblock

```c++
//if
void assembly_builder::visit(if_stmt_syntax &node)
{
    node.pred->accept(*this);
    if (!value_result) return;
    Value* cond = value_result;
    BasicBlock* then_body = BasicBlock::Create(context, "", current_function);
    if (node.else_body) {
        //有else部分
        BasicBlock* else_body = BasicBlock::Create(context, "", current_function);
        BasicBlock* next_body = BasicBlock::Create(context, "", current_function);
        builder.CreateCondBr(cond, then_body, else_body);
        
        builder.SetInsertPoint(then_body);
        node.then_body->accept(*this);
        builder.CreateBr(next_body);
      
        builder.SetInsertPoint(else_body);
        node.else_body->accept(*this);
        builder.CreateBr(next_body);
        
        builder.SetInsertPoint(next_body);
    } else {
        //无else部分，cond不满足时直接跳转到next块
        BasicBlock* next_body = BasicBlock::Create(context, "", current_function);
        builder.CreateCondBr(cond, then_body, next_body);
      
        builder.SetInsertPoint(then_body);
        node.then_body->accept(*this);
        builder.CreateBr(next_body);

        builder.SetInsertPoint(next_body);
    }
}
//while
void assembly_builder::visit(while_stmt_syntax &node)
{

    BasicBlock* cond = BasicBlock::Create(context, "", current_function);
    BasicBlock* body = BasicBlock::Create(context, "", current_function);
    BasicBlock* next = BasicBlock::Create(context, "", current_function);

    builder.CreateBr(cond);

    builder.SetInsertPoint(cond);
    node.pred->accept(*this);
    if (!value_result) return;
    Value* cond_v = value_result;
    builder.CreateCondBr(cond_v, body, next);

    builder.SetInsertPoint(body);
    node.body->accept(*this);
    builder.CreateBr(cond);

    builder.SetInsertPoint(next);
}
//条件判断
void assembly_builder::visit(cond_syntax &node)
{
    node.lhs->accept(*this);
    if (!value_result) return;
    Value* l = value_result;
    node.rhs->accept(*this);
    if (!value_result) return;
    Value* r = value_result;
    switch(node.op) {
        case relop::equal : 
            value_result = builder.CreateICmpEQ(l, r);
            return;
        case relop::greater :
            value_result = builder.CreateICmpSGT(l, r);
            return;
        case relop::greater_equal :
            value_result = builder.CreateICmpSGE(l, r);
            return;
        case relop::less :
            value_result = builder.CreateICmpSLT(l, r);
            return;
        case relop::less_equal :
            value_result = builder.CreateICmpSLE(l, r);
            return;
        case relop::non_equal :
            value_result = builder.CreateICmpNE(l, r);
            return;
        default:
            err.error(node.line, node.pos, "invalid opcode");
            error_flag = true;
            return;
    }
}
```

## 测试

测试过程中发现了c1r的ref版本的一个bug：对于最后为行注释的文件，其并非以\n结尾，从而无法被识别为注释

### 所有可能的报错

`error.c`

```c
void foo(){}
void foo(){}// function redefinition
const int a=1;
int b[a];// constexpr_expected && variable length array declaration is not allowed
int g;
const int b[2] = {g};// constexpr_expected && initializer element is not a compile-time constant
void main(){
	c = 1;// var is not define, use before define
	a = 2;// cannot assign to variable with const-qualified type 'const int'
	int d = a[2];// subscripted variable is not an array
	g = b;// array name cannot be used as value directly
	int e[-2];// declared as an array with a negative size
	int f[2] = {1,2,3};// excess elements in array initializer
	bar();// function is not declared
}
int c = 3 / (3 - 3 + a);//initializer should be const expression & since a is ignored, 3 is divided by 0

```

```shell
$ ./c1i ../test/error.c
Error at position 2:0 function foo has been defined
Error at position 4:6 constant expression expected
Error at position 6:18 constant expression expected
Error at position 6:10 variable b has been defined
Error at position 8:1 undefined variable c
Error at position 9:1 left value a is constant here
Error at position 10:9 a is not an array
Error at position 11:5 variable b is an array
Error at position 12:5 declare an array with zero or negtive size
Error at position 13:5 initializers' size exceeded array size
Error at position 14:1 no function named bar
Error at position 16:21 constant expression expected
Error at position 16:8 divided by zero
Semantic failed. Exiting.
```

### 递归计算斐波纳契数列

`fib.c`

```c
int fib_n;
int fib_result;

void fib()
{
	int n = fib_n;
	if(fib_n <= 1)
		fib_result = 1;
	else
	{
		fib_n = n - 1;
		fib();
		int result = fib_result;
		fib_n = n - 2;
		fib();
		fib_result = result + fib_result;
	}
}

void main() {
	input();
	fib_n = input_var;
	fib();
	output_var = fib_result;
	output();
}
```

```shell
$ ./c1i ../test/fib.c
9
55
```

程序运行如预期