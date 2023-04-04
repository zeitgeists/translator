#include "Token.hpp"

Token::Token() {
    this->tokenId = 0;
    this->identifierStr = "";
    this->operatorStr = "";
    this->numVal = 0;
}

Token::Token(const Token &token) {
    this->tokenId = token.tokenId;
    this->identifierStr = token.identifierStr;
    this->operatorStr = token.operatorStr;
    this->numVal = token.numVal;
}

Token& Token::operator=(const Token& token) {
    this->tokenId = token.tokenId;
    this->identifierStr = token.identifierStr;
    this->operatorStr = token.operatorStr;
    this->numVal = token.numVal;
    return *this;
}
