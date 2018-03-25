
#include "syntax_tree_builder.h"
#include <memory>

using namespace c1_recognizer;
using namespace c1_recognizer::syntax_tree;

syntax_tree_builder::syntax_tree_builder(error_reporter &_err) : err(_err) {}

antlrcpp::Any syntax_tree_builder::visitCompilationUnit(C1Parser::CompilationUnitContext *ctx)
{
    auto result = new assembly;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    ///////auto 
    for (auto child : ctx->children) {
        if (antlrcpp::is<C1Parser::DeclContext *>(child)) {
            /*
            ptr<assembly> ptr1;
            ptr1.reset(visit(child).as<assembly *>());
            for (auto vardef : ptr1->global_defs)
                result->global_defs.push_back(vardef);
            */
            auto ret = visit(child);
            for (auto def : *(ptr_list<var_def_stmt_syntax> *)ret) {
                result->global_defs.push_back(def);
            }
        }
        if (antlrcpp::is<C1Parser::FuncdefContext *>(child)) {
            ptr<func_def_syntax> ptr1;
            ptr1.reset(visit(child).as<func_def_syntax *>());
            result->global_defs.push_back(ptr1);
        }
    }
    return static_cast<assembly *>(result);
}

antlrcpp::Any syntax_tree_builder::visitDecl(C1Parser::DeclContext *ctx)
{
    /*
    if (auto decl = ctx->constdecl()) {
        return static_cast<assembly *>(visit(decl).as<assembly *>());
    }
    if (auto decl = ctx->vardecl()) {
        return static_cast<assembly *>(visit(decl).as<assembly *>());
    }
    */
    if (auto decl = ctx->constdecl()) {
        return visit(decl);
    }
    if (auto decl = ctx->vardecl()) {
        return visit(decl);
    }
}

antlrcpp::Any syntax_tree_builder::visitConstdecl(C1Parser::ConstdeclContext *ctx)
{
    /*
    auto result = new assembly;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    auto defs = ctx->constdef();
    for (auto def : defs) {
        ptr<global_def_syntax> def_;
        def_.reset(visit(def).as<var_def_stmt_syntax *>());
        result->global_defs.push_back(def_);
    }
    return static_cast<assembly *>(result);
    //*/
    //*
    auto defs = ctx->constdef();
    auto def_list = new ptr_list<var_def_stmt_syntax>;
    for (auto def : defs) {
        ptr<var_def_stmt_syntax> def_;
        def_.reset(visit(def).as<var_def_stmt_syntax *>());
        def_list->push_back(def_);
    }
    if (!ctx->Int()) {
        auto const_ = ctx->Const();
        err.warn(const_->getSymbol()->getLine(), const_->getSymbol()->getCharPositionInLine() + 6, "Type missing here. Assuming 'Int'");
    }
    return static_cast<ptr_list<var_def_stmt_syntax> *>(def_list);
    //*/
}

antlrcpp::Any syntax_tree_builder::visitConstdef(C1Parser::ConstdefContext *ctx)
{
    auto result = new var_def_stmt_syntax;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    auto id = ctx->Identifier();
    auto text = id->getSymbol()->getText();
    result->name = text;
    result->is_constant = true;
    if (ctx->LeftBrace()) {
        int length = ctx->Comma().size() + 1;
        auto expressions = ctx->exp();
        int lb = ctx->LeftBracket()->getSymbol()->getCharPositionInLine();
        int rb = ctx->RightBracket()->getSymbol()->getCharPositionInLine();
        if (rb - lb > 1) {
            result->array_length.reset(visit(expressions[0]).as<expr_syntax *>());
            for (size_t i = 1; i < expressions.size(); i++) {
                ptr<expr_syntax> expr;
                expr.reset(visit(expressions[i]).as<expr_syntax *>());
                result->initializers.push_back(expr);
            }
        } else {
            auto array_length = new literal_syntax;
            array_length->number = length;
            array_length->line = ctx->RightBracket()->getSymbol()->getLine();
            array_length->pos = rb;
            result->array_length.reset(array_length);
            for (auto expr : expressions) {
                ptr<expr_syntax> expr_;
                expr_.reset(visit(expr).as<expr_syntax *>());
                result->initializers.push_back(expr_);
            }
        }
        return static_cast<var_def_stmt_syntax *>(result);
    }
    if (ctx->Assign()) {
        auto expression = ctx->exp();
        ptr<expr_syntax> expr;
        expr.reset(visit(expression[0]).as<expr_syntax *>());
        result->initializers.push_back(expr);
        result->array_length = nullptr;
        return static_cast<var_def_stmt_syntax *>(result);
    }
}

antlrcpp::Any syntax_tree_builder::visitVardecl(C1Parser::VardeclContext *ctx)
{
    /*
    auto result = new assembly;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    auto defs = ctx->vardef();
    for (auto def : defs) {
        ptr<global_def_syntax> def_;
        def_.reset(visit(def).as<var_def_stmt_syntax *>());
        result->global_defs.push_back(def_);
    }
    return static_cast<assembly *>(result);
    //*/
    //*
    auto defs = ctx->vardef();
    auto def_list = new ptr_list<var_def_stmt_syntax>;
    for (auto def : defs) {
        ptr<var_def_stmt_syntax> def_;
        def_.reset(visit(def).as<var_def_stmt_syntax *>());
        def_list->push_back(def_);
    }
    return static_cast<ptr_list<var_def_stmt_syntax> *>(def_list);
    //*/
}

antlrcpp::Any syntax_tree_builder::visitVardef(C1Parser::VardefContext *ctx)
{
    auto result = new var_def_stmt_syntax;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    auto id = ctx->Identifier();
    auto text = id->getSymbol()->getText();
    result->name = text;
    result->is_constant = false;
    if (ctx->LeftBrace()) {
        int length = ctx->Comma().size() + 1;
        auto expressions = ctx->exp();
        int lb = ctx->LeftBracket()->getSymbol()->getCharPositionInLine();
        int rb = ctx->RightBracket()->getSymbol()->getCharPositionInLine();
        if (rb - lb > 1) {
            result->array_length.reset(visit(expressions[0]).as<expr_syntax *>());
            for (size_t i = 1; i < expressions.size(); i++) {
                ptr<expr_syntax> expr;
                expr.reset(visit(expressions[i]).as<expr_syntax *>());
                result->initializers.push_back(expr);
            }
        } else {
            auto array_length = new literal_syntax;
            array_length->number = length;
            array_length->line = ctx->RightBracket()->getSymbol()->getLine();
            array_length->pos = rb;
            result->array_length.reset(array_length);
            for (auto expr : expressions) {
                ptr<expr_syntax> expr_;
                expr_.reset(visit(expr).as<expr_syntax *>());
                result->initializers.push_back(expr_);
            }
        }
        return static_cast<var_def_stmt_syntax *>(result);
    }
    if (ctx->Assign()) {
        auto expression = ctx->exp();
        ptr<expr_syntax> expr;
        expr.reset(visit(expression[0]).as<expr_syntax *>());
        result->initializers.push_back(expr);
        result->array_length = nullptr;
        return static_cast<var_def_stmt_syntax *>(result);
    }
    if (ctx->LeftBracket()) {
        auto expression = ctx->exp();
        result->array_length.reset(visit(expression[0]).as<expr_syntax *>());
        return static_cast<var_def_stmt_syntax *>(result);
    }
    result->array_length = nullptr;
    return static_cast<var_def_stmt_syntax *>(result);
}

antlrcpp::Any syntax_tree_builder::visitFuncdef(C1Parser::FuncdefContext *ctx)
{
    auto result = new func_def_syntax;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    auto name = ctx->Identifier();
    auto text = name->getSymbol()->getText();
    result->name = text;
    auto body = ctx->block();
    result->body.reset(visit(body).as<block_syntax *>());
    return static_cast<func_def_syntax *>(result);
}

antlrcpp::Any syntax_tree_builder::visitBlock(C1Parser::BlockContext *ctx)
{
    auto result = new block_syntax;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    ///////auto 
    for (auto child : ctx->children) {
        if (antlrcpp::is<C1Parser::StmtContext *>(child)) {
            ptr<stmt_syntax> ptr1;
            ptr1.reset(visit(child).as<stmt_syntax *>());
            result->body.push_back(ptr1);
        }
        if (antlrcpp::is<C1Parser::DeclContext *>(child)) {
            /*
            if (auto decl = ((C1Parser::DeclContext *)child)->constdecl()) {
                auto defs = decl->constdef();
                for (auto def : defs) {
                    ptr<var_def_stmt_syntax> def_;
                    def_.reset(visit(def).as<var_def_stmt_syntax *>());
                    result->body.push_back(def_);
                }
            }
            if (auto decl = ((C1Parser::DeclContext *)child)->vardecl()) {
                auto defs = decl->vardef();
                for (auto def : defs) {
                    ptr<var_def_stmt_syntax> def_;
                    def_.reset(visit(def).as<var_def_stmt_syntax *>());
                    result->body.push_back(def_);
                }
            }
            */
            auto ret = visit(child);
            for (auto def : *(ptr_list<var_def_stmt_syntax> *)ret) {
                result->body.push_back(def);
            }
            
            /*
            ptr<assembly> ptrs;
            ptrs.reset(visit(child).as<assembly *>());
            for (auto ptr1 : ptrs->global_defs) {
                ptr<var_def_stmt_syntax> ptr2;
                ptr2.reset(((antlrcpp::Any)ptr1.get()).as<var_def_stmt_syntax *>());
                result->body.push_back(ptr2);
            }
            //*/
        }
    }
    return static_cast<block_syntax *>(result);
}

antlrcpp::Any syntax_tree_builder::visitStmt(C1Parser::StmtContext *ctx)
{
    if (ctx->Assign()) {
        auto result = new assign_stmt_syntax;
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        auto leftvalue = ctx->lval();
        result->target.reset(visit(leftvalue).as<lval_syntax *>());
        auto value = ctx->exp();
        result->value.reset(visit(value).as<expr_syntax *>());
        return static_cast<stmt_syntax *>(result);
    }
    if (auto id = ctx->Identifier()) {
        auto result = new func_call_stmt_syntax;
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        auto text = id->getSymbol()->getText();
        result->name = text;
        return static_cast<stmt_syntax *>(result);
    }
    if (ctx->If()) {
        auto result = new if_stmt_syntax;
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        auto cond = ctx->cond();
        result->pred.reset(visit(cond).as<cond_syntax *>());
        auto stmt = ctx->stmt();
        if (ctx->Else()) {
            result->then_body.reset(visit(stmt[0]).as<stmt_syntax *>());
            result->else_body.reset(visit(stmt[1]).as<stmt_syntax *>());
        } else {
            result->then_body.reset(visit(stmt[0]).as<stmt_syntax *>());
        }
        return static_cast<stmt_syntax *>(result);
    }
    if (ctx->While()) {
        auto result = new while_stmt_syntax;
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        auto cond = ctx->cond();
        result->pred.reset(visit(cond).as<cond_syntax *>());
        auto stmt = ctx->stmt();
        result->body.reset(visit(stmt[0]).as<stmt_syntax *>());
        return static_cast<stmt_syntax *>(result);
    }
    if (ctx->SemiColon()) {
        auto result = new empty_stmt_syntax;
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        return static_cast<stmt_syntax *>(result);
    }
    if (auto block = ctx->block()) {
        return static_cast<stmt_syntax *>(visit(block).as<block_syntax *>());
    }
}

antlrcpp::Any syntax_tree_builder::visitLval(C1Parser::LvalContext *ctx)
{
    auto identifier = ctx->Identifier();
    auto result = new lval_syntax;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    auto text = identifier->getSymbol()->getText();
    result->name = text;
    if (ctx->LeftBracket()) {
        auto index = ctx->exp();
        result->array_index.reset(visit(index).as<expr_syntax *>());
    }
    return static_cast<lval_syntax *>(result);
}

antlrcpp::Any syntax_tree_builder::visitCond(C1Parser::CondContext *ctx)
{
    auto expressions = ctx->exp();
    auto result = new cond_syntax;
    result->line = ctx->getStart()->getLine();
    result->pos = ctx->getStart()->getCharPositionInLine();
    result->lhs.reset(visit(expressions[0]).as<expr_syntax *>());
    if (ctx->Greater())
        result->op = relop::greater;
    if (ctx->GreaterEqual()) 
        result->op = relop::greater_equal;
    if (ctx->Equal()) 
        result->op = relop::equal;
    if (ctx->LessEqual())
        result->op = relop::less_equal;
    if (ctx->Less())
        result->op = relop::less;
    if (ctx->NonEqual()) 
        result->op = relop::non_equal;
    result->rhs.reset(visit(expressions[1]).as<expr_syntax *>());
    return static_cast<cond_syntax *>(result);
}

// Returns antlrcpp::Any, which is constructable from any type.
// However, you should be sure you use the same type for packing and depacking the `Any` object.
// Or a std::bad_cast exception will rise.
// This function always returns an `Any` object containing a `expr_syntax *`.
antlrcpp::Any syntax_tree_builder::visitExp(C1Parser::ExpContext *ctx)
{
    // Get all sub-contexts of type `exp`.
    auto expressions = ctx->exp();
    // Two sub-expressions presented: this indicates it's a expression of binary operator, aka `binop`.
    if (expressions.size() == 2)
    {
        auto result = new binop_expr_syntax;
        // Set line and pos.
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        // visit(some context) is equivalent to calling corresponding visit method; dispatching is done automatically
        // by ANTLR4 runtime. For this case, it's equivalent to visitExp(expressions[0]).
        // Use reset to set a new pointer to a std::shared_ptr object. DO NOT use assignment; it won't work.
        // Use `.as<Type>()' to get value from antlrcpp::Any object; notice that this Type must match the type used in
        // constructing the Any object, which is constructed from (usually pointer to some derived class of
        // syntax_node, in this case) returning value of the visit call.
        result->lhs.reset(visit(expressions[0]).as<expr_syntax *>());
        // Check if each token exists.
        // Returnd value of the calling will be nullptr (aka NULL in C) if it isn't there; otherwise non-null pointer.
        if (ctx->Plus())
            result->op = binop::plus;
        if (ctx->Minus())
            result->op = binop::minus;
        if (ctx->Multiply())
            result->op = binop::multiply;
        if (ctx->Divide())
            result->op = binop::divide;
        if (ctx->Modulo())
            result->op = binop::modulo;
        result->rhs.reset(visit(expressions[1]).as<expr_syntax *>());
        return static_cast<expr_syntax *>(result);
    }
    // Otherwise, if `+` or `-` presented, it'll be a `unaryop_expr_syntax`.
    if (ctx->Plus() || ctx->Minus())
    {
        auto result = new unaryop_expr_syntax;
        result->line = ctx->getStart()->getLine();
        result->pos = ctx->getStart()->getCharPositionInLine();
        if (ctx->Plus())
            result->op = unaryop::plus;
        if (ctx->Minus())
            result->op = unaryop::minus;
        result->rhs.reset(visit(expressions[0]).as<expr_syntax *>());
        return static_cast<expr_syntax *>(result);
    }
    // In the case that `(` exists as a child, this is an expression like `'(' expressions[0] ')'`.
    if (ctx->LeftParen())
        return visit(expressions[0]); // Any already holds expr_syntax* here, no need for dispatch and re-patch with casting.
    // If `Number` exists as a child, we can say it's a literal integer expression.
    if (auto number = ctx->Number())
    {
        auto result = new literal_syntax;
        result->line = number->getSymbol()->getLine();
        result->pos = number->getSymbol()->getCharPositionInLine();
        auto text = number->getSymbol()->getText();
        if (text[0] == '0' && text[1] == 'x')              // Hexadecimal
            result->number = std::stoi(text, nullptr, 16); // std::stoi will eat '0x'
        else                                               // Decimal
            result->number = std::stoi(text, nullptr, 10);
        return static_cast<expr_syntax *>(result);
    }
    if (auto leftvalue = ctx->lval()) {
        return static_cast<expr_syntax *>(visit(leftvalue).as<lval_syntax *>());
    }
}

ptr<syntax_tree_node> syntax_tree_builder::operator()(antlr4::tree::ParseTree *ctx)
{
    auto result = visit(ctx);
    if (result.is<syntax_tree_node *>())
        return ptr<syntax_tree_node>(result.as<syntax_tree_node *>());
    if (result.is<assembly *>())
        return ptr<syntax_tree_node>(result.as<assembly *>());
    if (result.is<global_def_syntax *>())
        return ptr<syntax_tree_node>(result.as<global_def_syntax *>());
    if (result.is<func_def_syntax *>())
        return ptr<syntax_tree_node>(result.as<func_def_syntax *>());
    if (result.is<cond_syntax *>())
        return ptr<syntax_tree_node>(result.as<cond_syntax *>());
    if (result.is<expr_syntax *>())
        return ptr<syntax_tree_node>(result.as<expr_syntax *>());
    if (result.is<binop_expr_syntax *>())
        return ptr<syntax_tree_node>(result.as<binop_expr_syntax *>());
    if (result.is<unaryop_expr_syntax *>())
        return ptr<syntax_tree_node>(result.as<unaryop_expr_syntax *>());
    if (result.is<lval_syntax *>())
        return ptr<syntax_tree_node>(result.as<lval_syntax *>());
    if (result.is<literal_syntax *>())
        return ptr<syntax_tree_node>(result.as<literal_syntax *>());
    if (result.is<stmt_syntax *>())
        return ptr<syntax_tree_node>(result.as<stmt_syntax *>());
    if (result.is<var_def_stmt_syntax *>())
        return ptr<syntax_tree_node>(result.as<var_def_stmt_syntax *>());
    if (result.is<assign_stmt_syntax *>())
        return ptr<syntax_tree_node>(result.as<assign_stmt_syntax *>());
    if (result.is<func_call_stmt_syntax *>())
        return ptr<syntax_tree_node>(result.as<func_call_stmt_syntax *>());
    if (result.is<block_syntax *>())
        return ptr<syntax_tree_node>(result.as<block_syntax *>());
    if (result.is<if_stmt_syntax *>())
        return ptr<syntax_tree_node>(result.as<if_stmt_syntax *>());
    if (result.is<while_stmt_syntax *>())
        return ptr<syntax_tree_node>(result.as<while_stmt_syntax *>());
    return nullptr;
}
