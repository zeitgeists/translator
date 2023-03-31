#pragma once

#include <memory>
#include <stdio.h>
#include "Lexer.hpp"
#include "AST.hpp"

namespace Parser {

static int CurTok;
int getNextToken();

std::unique_ptr<ExprAST> LogError(std::string str);
std::unique_ptr<PrototypeAST> LogErrorP(std::string str);

std::unique_ptr<ExprAST> ParseNumberExpr();

std::unique_ptr<ExprAST> ParseIdentifierExpr();

std::unique_ptr<ExprAST> ParseOperation();

std::unique_ptr<ExprAST> ParsePrimary();

std::unique_ptr<ExprAST> ParseExpressionTail(std::unique_ptr<ExprAST> LHS);

std::unique_ptr<ExprAST> ParseExpression();

std::unique_ptr<FunctionAST> ParseTopLevelExpr();
}
