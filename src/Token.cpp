#include "Token.hpp"

Token::Token() {
    this->type = Illegal;
    this->subtype = None;
    this->str = "";
}

Token::Token (TokenType t, TokenSubtype st) {
    this->type = t;
    this->subtype = st;
    this->str = "";
}

Token::Token(const Token &t) {
    this->type = t.type;
    this->subtype = t.subtype;
    this->str = t.str;
}

Token& Token::operator=(const Token& t) {
    this->type = t.type;
    this->subtype = t.subtype;
    this->str = t.str;
    return *this;
}

std::string Token::typeToStr() {
    std::string str;
    switch (this->type) {
    case Eof:
        str = "Eof";
        break;
    case Keyword:
        str = "Keyword";
        break;
    case Operator:
        str = "Operator";
        break;
    case Identifier:
        str = "Identifier";
        break;
    case Number:
        str = "Number";
        break;
    case Separator:
        str = "Separator";
        break;
    case Whitespace:
        str = "Whitespace";
        break;
    case Comment:
        str = "Comment";
        break;
    case Illegal:
        str = "Illegal";
        break;
    }
    return str;
}

std::string Token::subtypeToStr() {
    std::string str;
    switch (this->subtype) {
    case Extern:
        str = "Extern";
        break;
    case Def:
        str = "Def";
        break;
    case Plus:
        str = "Plus";
        break;
    case Minus:
        str = "Minus";
        break;
    case Multiply:
        str = "Multiply";
        break;
    case Divide:
        str = "Divide";
        break;
    case Comma:
        str = "Comma";
        break;
    case Semicolon:
        str = "Semicolon";
        break;
    case LParen:
        str = "LParen";
        break;
    case RParen:
        str = "RParen";
        break;
    case LCurly:
        str = "LCurly";
        break;
    case RCurly:
        str = "RCurly";
        break;
    case LineComment:
        str = "LineComment";
        break;
    case None:
        str = "None";
        break;
    }
    return str;
}

void TokenSet::Extern (Token& t) {
    t.type = Token::Keyword;
    t.subtype = Token::Extern;
}

void TokenSet::Def (Token& t) {
    t.type = Token::Keyword;
    t.subtype = Token::Def;
}

void TokenSet::Identifier (Token& t) {
    t.type = Token::Identifier;
    t.subtype = Token::None;
}

void TokenSet::Number (Token& t){
    t.type = Token::Number;
    t.subtype = Token::None;
}

void TokenSet::Plus (Token& t) {
    t.type = Token::Operator;
    t.subtype = Token::Plus;
}

void TokenSet::Minus (Token& t) {
    t.type = Token::Operator;
    t.subtype = Token::Minus;
}

void TokenSet::Multiply (Token& t) {
    t.type = Token::Operator;
    t.subtype = Token::Multiply;
}

void TokenSet::Divide (Token& t) {
    t.type = Token::Operator;
    t.subtype = Token::Divide;
}

void TokenSet::Comma (Token& t) {
    t.type = Token::Separator;
    t.subtype = Token::Comma;
}

void TokenSet::Semicolon (Token& t) {
    t.type = Token::Separator;
    t.subtype = Token::Semicolon;
}

void TokenSet::LParen (Token& t) {
    t.type = Token::Separator;
    t.subtype = Token::LParen;
}

void TokenSet::RParen (Token& t) {
    t.type = Token::Separator;
    t.subtype = Token::RParen;
}

void TokenSet::LCurly (Token& t) {
    t.type = Token::Separator;
    t.subtype = Token::LCurly;
}

void TokenSet::RCurly (Token& t) {
    t.type = Token::Separator;
    t.subtype = Token::RCurly;
}

void TokenSet::Whitespace (Token& t) {
    t.type = Token::Whitespace;
    t.subtype = Token::None;
}

void TokenSet::LineComment (Token& t) {
    t.type = Token::Comment;
    t.subtype = Token::LineComment;
}

void TokenSet::Eof (Token& t) {
    t.type = Token::Eof;
    t.subtype = Token::None;
}

void TokenSet::Illegal (Token& t) {
    t.type = Token::Illegal;
    t.subtype = Token::None;
}
