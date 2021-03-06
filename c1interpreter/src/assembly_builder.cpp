
#include "assembly_builder.h"

#include <vector>

using namespace llvm;
using namespace c1_recognizer::syntax_tree;

void assembly_builder::visit(assembly &node)
{
    for (auto global_defs : node.global_defs) {
        global_defs->accept(*this);
    }
}

void assembly_builder::visit(func_def_syntax &node)
{
    if (functions.find(node.name) != functions.end()) {
        err.error(node.line, node.pos, "function " + node.name + " has been defined");
        error_flag = true;
        return;
    }
    auto func = Function::Create(FunctionType::get(Type::getVoidTy(context), std::vector<Type *>(), false),
                                 GlobalValue::LinkageTypes::ExternalLinkage,
                                 node.name.data(), module.get());
    functions[node.name] = func;
    current_function = func;
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

void assembly_builder::visit(lval_syntax &node)
{///////////////////////////////////////////////////////////
    if (lval_as_rval) {
        if (constexpr_expected) {
            err.error(node.line, node.pos, "constant expression expected");
            error_flag = true;
            value_result = nullptr;
            return;
        }

        std::tuple<Value*, bool, bool> v = lookup_variable(node.name);
        
        if (std::get<0>(v) == nullptr){
            err.error(node.line, node.pos, "undefined variable " + node.name);
            error_flag = true;
            value_result = nullptr;
            return;
        }
        if (node.array_index) {
            if (std::get<2>(v) == false) {
                err.error(node.line, node.pos, node.name + " is not an array");
                error_flag = true;
                value_result = nullptr;
                return;
            }
            node.array_index->accept(*this);
            Value* index[] = {ConstantInt::get(context, APInt(32, 0)), value_result};
            value_result = builder.CreateInBoundsGEP(std::get<0>(v), index);
            value_result = builder.CreateLoad(value_result);
        } else if (std::get<2>(v) == true) {
            err.error(node.line, node.pos, "variable " + node.name + " is an array");
            error_flag = true;
            value_result = nullptr;
            return;
        } else {
            value_result = builder.CreateLoad(std::get<0>(v));
        }
    } else {

        std::tuple<Value*, bool, bool> v = lookup_variable(node.name);

        if (std::get<0>(v) == nullptr){
            err.error(node.line, node.pos, "undefined variable " + node.name);
            error_flag = true;
            value_result = nullptr;
            return;
        }

        if (std::get<1>(v)) {
            err.error(node.line, node.pos, "left value " + node.name + " is constant here");
            error_flag = true;
            value_result = nullptr;
            return;
        }
        if (node.array_index) {
            if (std::get<2>(v) == false) {
                err.error(node.line, node.pos, node.name + " is not an array");
                error_flag = true;
                value_result = nullptr;
                return;
            }
            lval_as_rval = true;
            node.array_index->accept(*this);
            lval_as_rval = false;
            //Value* index = value_result;
            Value* index[] = {ConstantInt::get(context, APInt(32, 0)), value_result};
            value_result = builder.CreateInBoundsGEP(std::get<0>(v), index);
        } else {
            if (std::get<2>(v) == true) {
                err.error(node.line, node.pos, node.name + " is an array");
                error_flag = true;
                value_result = nullptr;
                return;
            }
            value_result = std::get<0>(v);
        }

    }
}

void assembly_builder::visit(literal_syntax &node)
{
    if (constexpr_expected) {
        const_result = node.number;
    } else {
        value_result = ConstantInt::get(context, APInt(32, node.number));
    }
}

void assembly_builder::visit(var_def_stmt_syntax &node)
{
    if (in_global) {
        if (node.array_length != nullptr) {
            constexpr_expected = true;
            node.array_length->accept(*this);
            constexpr_expected = false;
            int size = const_result;
            if (size <= 0) {
                err.error(node.line, node.pos, "declare an array with zero or negtive size");
                error_flag = true;
                return;
            }
            if (node.initializers.size() > size) {
                err.error(node.line, node.pos, "initializers' size exceeded array size");
                error_flag = true;
                return;
            }
            int i = 0;
            constexpr_expected = true;
            std::vector<Constant*> init_v;
            //put each init value into a vector
            for (auto initializer : node.initializers) {
                initializer->accept(*this);
                init_v.push_back(ConstantInt::get(Type::getInt32Ty(module->getContext()), const_result));
                i++;
                //init_value[i] = ConstantInt::get(Type::getInt32Ty(module->getContext()), const_result);
            }
            constexpr_expected = false;
            auto zero = ConstantInt::get(context, APInt(32, 0));
            for (; i < size; i++) {
                init_v.push_back(zero);
            }
            // uses the vector to init a constant array for storage of init value
            // /auto init = ConstantArray::get(ArrayType::get(Type::getInt32Ty(module->getContext()), size), init_v);
            //ConstantArray init(ArrayType::get(Type::getInt32Ty(module->getContext()), size), init_v);
            Constant* init = ConstantArray::get(ArrayType::get(Type::getInt32Ty(module->getContext()), size), init_v);
            //build the global variable
            GlobalVariable* v = new GlobalVariable(*module,
                                                    ArrayType::get(Type::getInt32Ty(module->getContext()), size),
                                                    node.is_constant,
                                                    GlobalValue::ExternalLinkage,
                                                    init,
                                                    node.name);
            if (!declare_variable(node.name, v, node.is_constant, true)) {
                err.error(node.line, node.pos, "variable " + node.name + " has been defined");
                error_flag = true;
                return;
            }
        } else {
            GlobalVariable* v;
            if (node.initializers.size()) {
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

                v = new GlobalVariable(*module,
                                        Type::getInt32Ty(module->getContext()),
                                        node.is_constant,
                                        GlobalValue::ExternalLinkage,
                                        ConstantInt::get(context, APInt(32, 0)),
                                        node.name);
            }
            if (!declare_variable(node.name, v, node.is_constant, false)) {
                err.error(node.line, node.pos, "variable " + node.name + " has been defined");
                error_flag = true;
                return;
            }
        }
    } else {
        bool is_array = node.array_length != nullptr;
        if (is_array) {
            constexpr_expected = true;
            node.array_length->accept(*this);
            constexpr_expected = false;
            int v_l_ = const_result;
            if (v_l_ <= 0) {
                err.error(node.line, node.pos, "declare an array with zero or negtive size");
                error_flag = true;
                return;
            }
            //Value* v_l = ConstantInt::get(context, APInt(32, v_l_));

            Value* v = builder.CreateAlloca(ArrayType::get(Type::getInt32Ty(module->getContext()), v_l_), nullptr, node.name);

            //Value* v = builder.CreateAlloca(Type::getInt32Ty(context), v_l, node.name);
            if (!declare_variable(node.name, v, node.is_constant, is_array)) {
                err.error(node.line, node.pos, "variable " + node.name + " has been defined");
                error_flag = true;
                return;
            }
            if (node.initializers.size() > v_l_) {
                err.error(node.line, node.pos, "initializers' size exceeded array size");
                error_flag = true;
                return;
            }
            if (node.initializers.size()) {
                int i = 0;
                for (auto initializer : node.initializers) {
                    Value* index[] = {ConstantInt::get(context, APInt(32, 0)), ConstantInt::get(context, APInt(32, i))};
                    //Value* index = ConstantInt::get(context, APInt(32, i));
                    Value* ptr = builder.CreateGEP(v, index);
                    initializer->accept(*this);
                    if (value_result == nullptr) {
                        return;
                    }
                    builder.CreateStore(value_result, ptr);
                    i++;
                }
                Value* zero = ConstantInt::get(context, APInt(32, 0));
                while (i < v_l_) {
                    Value* index[] = {ConstantInt::get(context, APInt(32, 0)), ConstantInt::get(context, APInt(32, i))};
                    //Value* index = ConstantInt::get(context, APInt(32, i));
                    Value* ptr = builder.CreateInBoundsGEP(v, index);
                    builder.CreateStore(zero, ptr);
                    i++;
                }
            }
        } else {
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
    //node.
}

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

void assembly_builder::visit(func_call_stmt_syntax &node)
{
    if (functions.find(node.name) == functions.end()) {
        err.error(node.line, node.pos, "no function named " + node.name);
        error_flag = true;
        return;
    }
    builder.CreateCall(functions[node.name], {});
}

void assembly_builder::visit(block_syntax &node)
{
    enter_scope();
    for (auto stmt : node.body) {
        stmt->accept(*this);
    }
    exit_scope();
}

void assembly_builder::visit(if_stmt_syntax &node)
{
    node.pred->accept(*this);
    if (!value_result) return;
    Value* cond = value_result;
    BasicBlock* then_body = BasicBlock::Create(context, "", current_function);
    if (node.else_body) {
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
        BasicBlock* next_body = BasicBlock::Create(context, "", current_function);
        builder.CreateCondBr(cond, then_body, next_body);
        builder.SetInsertPoint(then_body);
        node.then_body->accept(*this);
        builder.CreateBr(next_body);

        builder.SetInsertPoint(next_body);
    }
    
}

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

void assembly_builder::visit(empty_stmt_syntax &node)
{
}
