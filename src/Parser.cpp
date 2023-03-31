#include "Parser.hpp"
#include <string>

namespace Parser {

int getNextToken() {
    return CurTok = Lexer::gettok();
}

std::unique_ptr<ExprAST> LogError(std::string str) {
    fprintf(stderr, "Error: %s\n", str.c_str());
    return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(std::string str) {
    LogError(str);
    return nullptr;
}

std::unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = std::make_unique<NumberExprAST>(NumVal);
    getNextToken(); // consume the number
    return Result;
}

std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string IdName = IdentifierStr;

    getNextToken(); // eat identifier.

    if (CurTok != '(') // Simple variable ref.
        return std::make_unique<VariableExprAST>(IdName);

    // Call.
    getNextToken(); // eat (
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (CurTok != ')') {
        while (true) {
            if (auto Arg = ParseExpression()) Args.push_back(std::move(Arg));
            else return nullptr;
            if (CurTok == ')') break;
            if (CurTok != ',') return LogError("Expected ')' or ',' in argument list");
            getNextToken();
        }
    }

    // Eat the ')'.
    getNextToken();

    return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

std::unique_ptr<ExprAST> ParseOperationTerm() {
    LogError(OperatorStr);
    LogError(IdentifierStr);
    LogError(std::to_string(NumVal));
    switch (CurTok) {
    case tok_identifier:
        return ParseIdentifierExpr();
    case tok_number:
        return ParseNumberExpr();
    default:
        return LogError("unknown token when expecting an OPERATION_TERM");
    }
}

std::unique_ptr<ExprAST> ParseOperation() {
    if (CurTok == '(') {
        getNextToken(); // consume '('
        auto result = ParseExpression();
        getNextToken(); // consume ')'
        return result;
    }
    auto LHS = ParseOperationTerm();
    if (!LHS) return nullptr;

    getNextToken();
    if (CurTok != tok_operator_1) return LHS;

    std::string opStr = OperatorStr;
    auto RHS = ParseOperationTerm();
    // Merge LHS/RHS.
    return std::make_unique<OperatorAST>(opStr, std::move(LHS), std::move(RHS));
}

std::unique_ptr<ExprAST> ParsePrimary() {
    if (CurTok == '-') {
        //TODO: add negation
        return LogError("negation not implemented yet");
    }
    return ParseOperation();
}

std::unique_ptr<ExprAST> ParseExpressionTail(std::unique_ptr<ExprAST> LHS) {
    if (CurTok == ';' || CurTok == tok_eof) return LHS;
    if (CurTok != tok_operator_2) return LogError("Expected OPERATOR_2 in EXPRESSION_TAIL");

    std::string opStr = OperatorStr;
    auto RHS = ParsePrimary();
    if (!RHS) return nullptr;

    RHS = ParseExpressionTail(std::move(RHS));

    // Merge LHS/RHS.
    LHS = std::make_unique<OperatorAST>(opStr, std::move(LHS), std::move(RHS));
    return nullptr;
}

std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS) return nullptr;

    return ParseExpressionTail(std::move(LHS));
}

std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
  if (auto E = ParseExpression()) {
    // Make an anonymous proto.
    auto Proto = std::make_unique<PrototypeAST>("__anonymous_expr",
                                                std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}
}
