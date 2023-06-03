#pragma once
#include <string>
#include <fmt/core.h>

class Token {
public:
    enum TokenType {
        Keyword,
        Operator,
        Identifier,
        Number,
        Separator,
        Whitespace,
        Comment,
        Eof,
        Illegal
    };

    enum TokenSubtype {
        Extern,
        Def,
        Plus,
        Minus,
        Multiply,
        Divide,
        Comma,
        Semicolon,
        LParen,
        RParen,
        LCurly,
        RCurly,
        LineComment,
        None
    };

    TokenType type;
    TokenSubtype subtype;
    std::string str;

    Token ();
    Token (TokenType, TokenSubtype);
    Token (const Token&);
    Token& operator= (const Token&);

    std::string typeToStr();
    std::string subtypeToStr();
};

namespace TokenSet {
    void Extern (Token&);
    void Def (Token&);
    void Identifier (Token&);
    void Number (Token&);
    void Plus (Token&);
    void Minus (Token&);
    void Multiply (Token&);
    void Divide (Token&);
    void Comma (Token&);
    void Semicolon (Token&);
    void LParen (Token&);
    void RParen (Token&);
    void LCurly (Token&);
    void RCurly (Token&);
    void Whitespace (Token&);
    void LineComment (Token&);
    void Eof (Token&);
    void Illegal (Token&);
}
