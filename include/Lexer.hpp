#pragma once

#include "Token.hpp"

namespace Lexer {
// gettok - Return the next token from standard input.
// returns tokens [0-255] if it is an unknown character
int gettok();
int ReadNextChar();
}
