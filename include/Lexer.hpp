#pragma once

#include "Token.hpp"

#include <list>
#include <fstream>
#include <iostream>
#include <iomanip>

namespace Lexer {
    // gettok - Return the next token from standard input.
    // returns tokens [0-255] if it is an unknown character
    Token GetToken();
    Token LogToken(Token &token);
    Token MakeToken();
    int ReadNextChar();
    std::string TokenIdToStr(int id);
    void PrintLoggedTokens();
}

static std::fstream fin("/home/bluten/Projects/translator/input.txt", std::fstream::in);
