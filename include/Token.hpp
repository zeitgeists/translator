#pragma once
#include <string>

enum TokenId {
  tok_eof = -1,

  // commands
  tok_def = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5,

  tok_operator_1 = -6,
  tok_operator_2 = -7,
};

struct Token {
public:
    int tokenId;
    std::string identifierStr;   // Filled in if tok_identifier
    std::string operatorStr;     // Filled in if tok_operator
    double numVal;               // Filled in if tok_number
    Token();
    Token(const Token &token);
    Token& operator=(const Token &token);
};
