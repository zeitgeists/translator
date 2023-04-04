#include "Lexer.hpp"
#include <iomanip>

std::list<Token> loggedTokens;

int Lexer::ReadNextChar() {
    // fin >> std::noskipws >> ch;
    char ch;
    fin.get(ch);
    if (fin.eof()) return EOF;
    // std::cout << "Readed form file: " << ch << std::endl;
    return ch;
    //return getchar();
}

std::string Lexer::TokenIdToStr(int id) {
    std::string str;
    switch (id) {
    case tok_eof:
        str = "eof";
        break;
    case tok_def:
        str = "def";
        break;
    case tok_extern:
        str = "extern";
        break;
    case tok_identifier:
        str = "identifier";
        break;
    case tok_number:
        str = "number";
        break;
    case tok_operator_1:
        str = "operator_1";
        break;
    case tok_operator_2:
        str = "operator_2";
        break;
    default:
        str = id;
    }
    return str;
}

void Lexer::PrintLoggedTokens() {
    Token token;
    std::cout << "    id    | identifier | operator | number\n";
    while (!loggedTokens.empty()) {
        token = loggedTokens.front();
        loggedTokens.pop_front();
        std::cout << std::right << std::setw(10) << TokenIdToStr(token.tokenId)
            << "|" << std::setw(12) << token.identifierStr
            << "|" << std::setw(10) << token.operatorStr
            << "|" << std::setw(10) << token.numVal
            << std::endl;
    }
}

Token Lexer::GetToken() {
    Token token = MakeToken();
    loggedTokens.push_back(token);
    return token;
}

Token Lexer::MakeToken() {
    Token token;
    static int LastChar = ' ';

    // Skip any whitespace.
    while (isspace(LastChar))
        LastChar = ReadNextChar();

    if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
        token.identifierStr = LastChar;
        while (isalnum((LastChar = ReadNextChar())))
            token.identifierStr += LastChar;

        if (token.identifierStr == "def") {
            token.tokenId = tok_def;
        } else if (token.identifierStr == "extern") {
            token.tokenId = tok_extern;
        } else {
            token.tokenId = tok_identifier;
        }
        return token;
    }

    if (isdigit(LastChar) || LastChar == '.') {   // Number: [0-9.]+
        std::string NumStr;
        do {
            NumStr += LastChar;
            LastChar = ReadNextChar();
        } while (isdigit(LastChar) || LastChar == '.');

        token.numVal = strtod(NumStr.c_str(), 0);
        token.tokenId = tok_number;
        return token;
    }

    switch(LastChar) {
    case '+':
    case '-':
        token.operatorStr = LastChar;
        LastChar = ReadNextChar();
        token.tokenId = tok_operator_1;
        return token;
    case '*':
    case '/':
    case '%':
        token.operatorStr = LastChar;
        LastChar = ReadNextChar();
        token.tokenId = tok_operator_2;
        return token;
    default:
        break;
    }

    if (LastChar == '#') {
        // Comment until end of line.
        do
            LastChar = ReadNextChar();
        while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        if (LastChar != EOF) return GetToken();
    }

    // Check for end of file.  Don't eat the EOF.
    if (LastChar == EOF) {
        token.tokenId = tok_eof;
        return token;
    }

    // Otherwise, just return the character as its ascii value.
    token.tokenId = LastChar;
    LastChar = ReadNextChar();
    return token;
}
