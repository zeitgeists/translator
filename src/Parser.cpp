#include "Parser.hpp"
#include <memory>
#include <string>

Token Parser::GetCurrentToken() {
    return currentToken;
}

void Parser::GetNextToken() {
    currentToken = Lexer::GetToken();
}

std::unique_ptr<ExprAST> Parser::ParseTerm() {
    if (currentToken.tokenId == tok_number) {
        auto result = std::make_unique<NumberExprAST>(currentToken.numVal);
        GetNextToken(); // consume the number
        return result;

    } else if (currentToken.tokenId == '(') {
        GetNextToken(); // eat '('
        auto result = ParseExpression();
        GetNextToken(); // eat ')'
        return std::move(result);

    } else if (currentToken.tokenId == tok_identifier) {
        std::string IdName = currentToken.identifierStr;
        GetNextToken(); // eat identifier.
        if (currentToken.tokenId != '(') // Simple variable reference
            return std::make_unique<VariableExprAST>(IdName);

        // Call.
        GetNextToken(); // eat '('
        std::unique_ptr<std::vector<std::unique_ptr<ExprAST>>>
            Args = ParseArgList();
        if (!Args) return Logger::LogError("Error while parsing ArgList");
        GetNextToken(); // eat ')'
        return std::make_unique<CallExprAST>(IdName, std::move(Args));

    } else {
        return Logger::LogError("Unknown token when expecting an TERM");
    }
}

std::unique_ptr<ExprAST> Parser::ParseNegation() {
    // TODO: this check is bad, negation must have his own tokenId
    if (currentToken.tokenId == tok_operator_1 &&
            currentToken.operatorStr == "-") {
        // TODO: add negation
        return Logger::LogError("Negation not implemented yet");

    } else {
        return ParseTerm();
    }
}

std::unique_ptr<ExprAST> Parser::ParseOP_1_List() {
    auto LHS = ParseOP_2_List();
    if (!LHS) return nullptr;

    return ParseOP_1_ListTail(std::move(LHS));
}

std::unique_ptr<ExprAST> Parser::ParseOP_1_ListTail(std::unique_ptr<ExprAST> LHS) {
    if (currentToken.tokenId != tok_operator_1) return LHS;

    std::string opStr = currentToken.operatorStr;
    GetNextToken(); //consume operator_1
    auto RHS = ParseOP_2_List();
    // Merge LHS/RHS.
    auto new_LHS = std::make_unique<OperatorAST>(opStr, std::move(LHS), std::move(RHS));
    return ParseOP_1_ListTail(std::move(new_LHS));
}

std::unique_ptr<ExprAST> Parser::ParseOP_2_List() {
    auto LHS = ParseNegation();
    if (!LHS) return nullptr;

    return ParseOP_2_ListTail(std::move(LHS));
}

std::unique_ptr<ExprAST> Parser::ParseOP_2_ListTail(std::unique_ptr<ExprAST> LHS) {
    if (currentToken.tokenId != tok_operator_2) return LHS;

    std::string opStr = currentToken.operatorStr;
    GetNextToken(); //consume operator_2
    auto RHS = ParseNegation();
    // Merge LHS/RHS.
    auto new_LHS = std::make_unique<OperatorAST>(opStr, std::move(LHS), std::move(RHS));
    return ParseOP_2_ListTail(std::move(new_LHS));
}

std::unique_ptr<ExprAST> Parser::ParseExpression() {
    return ParseOP_1_List();
}

std::unique_ptr<std::vector<std::string>> Parser::ParseParamList() {
    auto Params = std::make_unique<std::vector<std::string>>();
    if (currentToken.tokenId == tok_identifier) {
        Params->push_back(currentToken.identifierStr);
        GetNextToken(); // eat identifier.
        return ParseParamListTail(std::move(Params));
    } else {
        return std::move(Params);
    }
}
std::unique_ptr<std::vector<std::string>> Parser::ParseParamListTail(
            std::unique_ptr<std::vector<std::string>> Params) {
    if (currentToken.tokenId != ',') return std::move(Params);

    GetNextToken(); // eat ','

    if (currentToken.tokenId == tok_identifier) {
        Params->push_back(currentToken.identifierStr);
        GetNextToken(); // eat identifier.
        return ParseParamListTail(std::move(Params));
    } else {
        return nullptr;
    }
}

std::unique_ptr<std::vector<std::unique_ptr<ExprAST>>> Parser::ParseArgList() {
    auto Args = std::make_unique<std::vector<std::unique_ptr<ExprAST>>>();

    auto arg = ParseExpression();
    if (!arg) return std::move(Args);

    Args->push_back(std::move(arg));
    return ParseArgListTail(std::move(Args));
}

std::unique_ptr<std::vector<std::unique_ptr<ExprAST>>> Parser::ParseArgListTail(
        std::unique_ptr<std::vector<std::unique_ptr<ExprAST>>> Args) {
    if (currentToken.tokenId != ',') return std::move(Args);

    GetNextToken(); // eat ','

    if (auto arg = ParseExpression()) {
        Args->push_back(std::move(arg));
        return ParseArgListTail(std::move(Args));
    } else {
        return nullptr;
    }
}

std::unique_ptr<PrototypeAST> Parser::ParsePrototype() {
    if (currentToken.tokenId != tok_identifier)
        return Logger::LogErrorP("Expected function name in prototype");

    std::string FnName = currentToken.identifierStr;
    Parser::GetNextToken();

    if (currentToken.tokenId != '(')
        return Logger::LogErrorP("Expected '(' in prototype");

    Parser::GetNextToken(); // eat '('

    std::unique_ptr<std::vector<std::string>>
        Params = ParseParamList();
    if (!Params) return Logger::LogErrorP("Error while parsing ParamList");

    if (currentToken.tokenId != ')') {
        Lexer::PrintLoggedTokens();
        return Logger::LogErrorP("Expected ')' in prototype");
    }

    // success.
    Parser::GetNextToken(); // eat ')'.

    return std::make_unique<PrototypeAST>(FnName, std::move(Params));
}

std::unique_ptr<PrototypeAST> Parser::ParseExtern() {
    Parser::GetNextToken(); // eat 'extern'
    return ParsePrototype();
}

std::unique_ptr<FunctionAST> Parser::ParseDefinition() {
    Parser::GetNextToken(); // eat def.
    auto Prototype = ParsePrototype();
    if (!Prototype) return nullptr;

    if (auto E = Parser::ParseExpression())
        return std::make_unique<FunctionAST>(std::move(Prototype), std::move(E));
    return nullptr;
}

std::unique_ptr<FunctionAST> Parser::ParseTopLevelExpr() {
    if (auto E = ParseExpression()) {
        // Make an anonymous proto.
        auto Proto = std::make_unique<PrototypeAST>("__anonymous_expr",
                                std::make_unique<std::vector<std::string>>());
        return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }
    return nullptr;
}
