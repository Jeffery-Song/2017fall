
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include <memory>

using namespace llvm;

int main()
{
    LLVMContext context;
    IRBuilder<> builder(context);

    auto module = new Module("fib-gen", context);
    std::map<std::string, Value*> NamedValues_fib;

    auto fib = Function::Create(FunctionType::get(Type::getInt32Ty(context), std::vector<Type *>(1, Type::getInt32Ty(context)), false),
                                 GlobalValue::LinkageTypes::ExternalLinkage,
                                 "fib", module);
    for (auto &Arg : fib->args()) {// only one parameter here
        Arg.setName("n");
        NamedValues_fib["n"] = &Arg;
    }
    auto fib_entry = BasicBlock::Create(context, "", fib);
    builder.SetInsertPoint(fib_entry);
    
    auto stack_var_ret = builder.CreateAlloca(Type::getInt32Ty(context));
    stack_var_ret->setAlignment(4);
    auto stack_var_n = builder.CreateAlloca(Type::getInt32Ty(context));
    stack_var_n->setAlignment(4);

    builder.CreateStore(NamedValues_fib["n"], stack_var_n)->setAlignment(4);
    auto var_n = builder.CreateLoad(stack_var_n, "");
    var_n->setAlignment(4);
    Value* const_zero = ConstantInt::get(context, APInt(32, 0));
    auto var_n_eq_0 = builder.CreateICmpEQ(var_n, const_zero);
    auto bb_n_eq_0 = BasicBlock::Create(context, "", fib);
    auto bb_n_neq_0 = BasicBlock::Create(context, "", fib);
    auto bb_n_eq_1 = BasicBlock::Create(context, "", fib);
    auto bb_n_neq_1 = BasicBlock::Create(context, "", fib);
    auto bb_ret = BasicBlock::Create(context, "", fib);
    builder.CreateCondBr(var_n_eq_0, bb_n_eq_0, bb_n_neq_0);

    builder.SetInsertPoint(bb_n_eq_0);
    builder.CreateStore(const_zero, stack_var_ret)->setAlignment(4);
    builder.CreateBr(bb_ret);

    builder.SetInsertPoint(bb_n_neq_0);
    Value* const_one = ConstantInt::get(context, APInt(32, 1));
    auto var_n_eq_1 = builder.CreateICmpEQ(var_n, const_one);
    builder.CreateCondBr(var_n_eq_1, bb_n_eq_1, bb_n_neq_1);

    builder.SetInsertPoint(bb_n_eq_1);
    builder.CreateStore(const_one, stack_var_ret)->setAlignment(4);
    builder.CreateBr(bb_ret);

    builder.SetInsertPoint(bb_n_neq_1);
    auto var_n_sub_1 = builder.CreateNSWSub(var_n, const_one);
    auto var_fib_n_sub_1 = builder.CreateCall(fib, {var_n_sub_1});
    Value* const_two = ConstantInt::get(context, APInt(32, 2));
    auto var_n_sub_2 = builder.CreateNSWSub(var_n, const_two);
    auto var_fib_n_sub_2 = builder.CreateCall(fib, {var_n_sub_2});
    auto var_fib1_add_fib2 = builder.CreateNSWAdd(var_fib_n_sub_1, var_fib_n_sub_2);
    builder.CreateStore(var_fib1_add_fib2, stack_var_ret)->setAlignment(4);
    builder.CreateBr(bb_ret);


    builder.SetInsertPoint(bb_ret);
    auto var_ret = builder.CreateLoad(stack_var_ret);
    var_ret->setAlignment(4);
    builder.CreateRet(var_ret);

    //main
    auto f_main = Function::Create(FunctionType::get(Type::getInt32Ty(context), std::vector<Type *>(), false),
                                                     GlobalValue::LinkageTypes::ExternalLinkage,
                                                     "main", module);
    
    auto main_entry = BasicBlock::Create(context, "", f_main);
    builder.SetInsertPoint(main_entry);
    auto stack_var_x = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "x");
    stack_var_x->setAlignment(4);
    auto stack_var_i = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "i");
    stack_var_i->setAlignment(4);
    builder.CreateStore(const_zero, stack_var_x)->setAlignment(4);
    builder.CreateStore(const_zero, stack_var_i)->setAlignment(4);
    auto bb_main_ret = BasicBlock::Create(context, "", f_main);
    auto bb_main_for_cond = BasicBlock::Create(context, "", f_main, bb_main_ret);
    auto bb_main_for_body = BasicBlock::Create(context, "", f_main, bb_main_ret);
    builder.CreateBr(bb_main_for_cond);

    builder.SetInsertPoint(bb_main_for_cond);
    auto var_main_i = builder.CreateLoad(stack_var_i);
    var_main_i->setAlignment(4);
    Value* const_ten = ConstantInt::get(context, APInt(32, 10));
    auto var_main_i_slt_10 = builder.CreateICmpSLT(var_main_i, const_ten);
    builder.CreateCondBr(var_main_i_slt_10, bb_main_for_body, bb_main_ret);

    builder.SetInsertPoint(bb_main_for_body);
    auto var_main_fib_i = builder.CreateCall(fib, {var_main_i});
    auto var_main_x = builder.CreateLoad(stack_var_x);
    var_main_x->setAlignment(4);
    auto var_main_x_add_fib = builder.CreateNSWAdd(var_main_x, var_main_fib_i);
    builder.CreateStore(var_main_x_add_fib, stack_var_x)->setAlignment(4);
    auto var_i_add_1 = builder.CreateNSWAdd(var_main_i, const_one);
    builder.CreateStore(var_i_add_1, stack_var_i)->setAlignment(4);;
    builder.CreateBr(bb_main_for_cond);

    builder.SetInsertPoint(bb_main_ret);
    auto var_main_ret = builder.CreateLoad(stack_var_x);
    var_main_ret->setAlignment(4);
    builder.CreateRet(var_main_ret);

    builder.ClearInsertionPoint();
    
    module->print(outs(), nullptr);
    delete module;
    return 0;
}