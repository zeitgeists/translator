#pragma once

#include <memory>
#include <stdio.h>
#include "Logger.hpp"
#include "Token.hpp"
#include "Lexer.hpp"
#include "AST.hpp"

static Token currentToken;

namespace Parser {
    void GetNextToken();
    std::unique_ptr<ExprAST> ParseTerm();
    std::unique_ptr<ExprAST> ParseNegation();
    std::unique_ptr<ExprAST> ParseOP_2_List();
    std::unique_ptr<ExprAST> ParseOP_2_ListTail(std::unique_ptr<ExprAST> LHS);
    std::unique_ptr<ExprAST> ParseOP_1_List();
    std::unique_ptr<ExprAST> ParseOP_1_ListTail(std::unique_ptr<ExprAST> LHS);
    std::unique_ptr<ExprAST> ParseExpression();
    std::unique_ptr<std::vector<std::string>> ParseParamList();
    std::unique_ptr<std::vector<std::string>> ParseParamListTail(
            std::unique_ptr<std::vector<std::string>> Params);
    std::unique_ptr<std::vector<std::unique_ptr<ExprAST>>> ParseArgList();
    std::unique_ptr<std::vector<std::unique_ptr<ExprAST>>> ParseArgListTail(
            std::unique_ptr<std::vector<std::unique_ptr<ExprAST>>> Args);
    std::unique_ptr<PrototypeAST> ParsePrototype();
    std::unique_ptr<PrototypeAST> ParseExtern();
    std::unique_ptr<FunctionAST> ParseDefinition();
    std::unique_ptr<FunctionAST> ParseTopLevelExpr();
    Token GetCurrentToken();
}
