#include "Parser.hpp"
#include <string>

Token Parser::GetCurrentToken() {
    return currentToken;
}

void Parser::GetNextToken() {
    currentToken = Lexer::GetToken();
}

std::unique_ptr<ExprAST> Parser::ParseNumberExpr() {
    auto Result = std::make_unique<NumberExprAST>(currentToken.numVal);
    GetNextToken(); // consume the number
    return Result;
}

std::unique_ptr<ExprAST> Parser::ParseIdentifierExpr() {
    std::string IdName = currentToken.identifierStr;

    GetNextToken(); // eat identifier.

    if (currentToken.tokenId != '(') // Simple variable ref.
        return std::make_unique<VariableExprAST>(IdName);

    // Call.
    GetNextToken(); // eat (
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (currentToken.tokenId != ')') {
        while (true) {
            if (auto Arg = ParseExpression()) Args.push_back(std::move(Arg));
            else return nullptr;
            if (currentToken.tokenId == ')') break;
            if (currentToken.tokenId != ',') return Logger::LogError("Expected ')' or ',' in argument list");
            GetNextToken();
        }
    }

    // Eat the ')'.
    GetNextToken();

    return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

std::unique_ptr<ExprAST> Parser::ParseOperationTerm() {
    switch (currentToken.tokenId) {
    case tok_identifier:
        return ParseIdentifierExpr();
    case tok_number:
        return ParseNumberExpr();
    default:
        return Logger::LogError("unknown token when expecting an OPERATION_TERM");
    }
}

std::unique_ptr<ExprAST> Parser::ParseOperation() {
    if (currentToken.tokenId == '(') {
        GetNextToken(); // consume '('
        auto result = ParseExpression();
        GetNextToken(); // consume ')'
        return result;
    }

    return ParseOperationTerm();
}

std::unique_ptr<ExprAST> Parser::ParsePrimary() {
    if (currentToken.operatorStr == "-") {
        //TODO: add negation
        return Logger::LogError("negation not implemented yet");
    }

    auto LHS = ParseOperation();
    if (!LHS) return nullptr;

    if (currentToken.tokenId != tok_operator_1) return LHS;

    std::string opStr = currentToken.operatorStr;
    GetNextToken(); //consume operator_1
    auto RHS = ParseOperation();
    // Merge LHS/RHS.
    return std::make_unique<OperatorAST>(opStr, std::move(LHS), std::move(RHS));
}

std::unique_ptr<ExprAST> Parser::ParseExpressionTail(std::unique_ptr<ExprAST> LHS) {
    if (currentToken.tokenId == ')') return LHS;
    if (currentToken.tokenId == ';' || currentToken.tokenId == tok_eof){
        GetNextToken();
        return LHS;
    }
    if (currentToken.tokenId != tok_operator_2) {
        Lexer::PrintLoggedTokens();
        LHS->ToStdOut("", false);
        return Logger::LogError("Expected OPERATOR_2 in EXPRESSION_TAIL");
    }

    std::string opStr = currentToken.operatorStr;
    GetNextToken();
    auto RHS = ParsePrimary();
    if (!RHS) return nullptr;

    RHS = ParseExpressionTail(std::move(RHS));

    // Merge LHS/RHS.
    return std::make_unique<OperatorAST>(opStr, std::move(LHS), std::move(RHS));

}

std::unique_ptr<ExprAST> Parser::ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS) return nullptr;

    return ParseExpressionTail(std::move(LHS));
}

std::unique_ptr<PrototypeAST> Parser::ParsePrototype() {
    if (currentToken.tokenId != tok_identifier)
        return Logger::LogErrorP("Expected function name in prototype");

    std::string FnName = currentToken.identifierStr;
    Parser::GetNextToken();

    if (currentToken.tokenId != '(')
        return Logger::LogErrorP("Expected '(' in prototype");

    std::vector<std::string> ArgNames;
    Parser::GetNextToken(); // eat '('
    while (currentToken.tokenId == tok_identifier) {
        ArgNames.push_back(currentToken.identifierStr);
        Parser::GetNextToken();
    }
    if (currentToken.tokenId != ')')
        return Logger::LogErrorP("Expected ')' in prototype");

    // success.
    Parser::GetNextToken(); // eat ')'.

    return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

std::unique_ptr<FunctionAST> Parser::ParseDefinition() {
    Parser::GetNextToken(); // eat def.
    auto Prototype = ParsePrototype();
    if (!Prototype) return nullptr;

    if (auto E = Parser::ParseExpression())
        return std::make_unique<FunctionAST>(std::move(Prototype), std::move(E));
    return nullptr;
}

std::unique_ptr<PrototypeAST> Parser::ParseExtern() {
    Parser::GetNextToken(); // eat 'extern'
    return ParsePrototype();
}

std::unique_ptr<FunctionAST> Parser::ParseTopLevelExpr() {
    if (auto E = ParseExpression()) {
        // Make an anonymous proto.
        auto Proto = std::make_unique<PrototypeAST>("__anonymous_expr",
                                            std::vector<std::string>());
        E->ToStdOut("", false);
        return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }
    return nullptr;
}
