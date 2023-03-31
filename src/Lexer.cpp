#include "Lexer.hpp"
#include <fstream>
#include <iostream>

static char ch;
static std::fstream fin("/home/bluten/Projects/translator/input.txt", std::fstream::in);

namespace Lexer {

int ReadNextChar() {
    // fin >> std::noskipws >> ch;
    // std::cout << "Readed form file: " << ch << std::endl;
    // return ch;
    return getchar();
}

int gettok() {
    static int LastChar = ' ';

    // Skip any whitespace.
    while (isspace(LastChar))
        LastChar = ReadNextChar();

    if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
        IdentifierStr = LastChar;
        while (isalnum((LastChar = ReadNextChar())))
            IdentifierStr += LastChar;

        if (IdentifierStr == "def") return tok_def;
        if (IdentifierStr == "extern") return tok_extern;
        return tok_identifier;
    }

    if (isdigit(LastChar) || LastChar == '.') {   // Number: [0-9.]+
        std::string NumStr;
        do {
            NumStr += LastChar;
            LastChar = ReadNextChar();
        } while (isdigit(LastChar) || LastChar == '.');

        NumVal = strtod(NumStr.c_str(), 0);
        return tok_number;
    }

    switch(LastChar) {
    case '+':
    case '-':
        OperatorStr = LastChar;
        return tok_operator_1;
    case '*':
    case '/':
    case '%':
        OperatorStr = LastChar;
        return tok_operator_2;
    default:
        break;
    }

    if (LastChar == '#') {
        // Comment until end of line.
        do
            LastChar = ReadNextChar();
        while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        if (LastChar != EOF) return gettok();
    }

    // Check for end of file.  Don't eat the EOF.
    if (LastChar == EOF) return tok_eof;

    // Otherwise, just return the character as its ascii value.
    int ThisChar = LastChar;
    LastChar = ReadNextChar();
    return ThisChar;
}
}
