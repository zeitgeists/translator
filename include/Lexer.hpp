#pragma once

#include "Token.hpp"
static std::string IdentifierStr;   // Filled in if tok_identifier
static std::string OperatorStr;     // Filled in if tok_operator
static double NumVal;               // Filled in if tok_number

namespace Lexer {
// gettok - Return the next token from standard input.
// returns tokens [0-255] if it is an unknown character
int gettok();
int ReadNextChar();
double getNumVal();
std::string getIdentifierStr();
std::string getOperatorStr();
}
