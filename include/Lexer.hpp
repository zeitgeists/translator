#pragma once

#include "Token.hpp"

#include <cstdint>
#include <list>
#include <fstream>
#include <iostream>
#include <fmt/core.h>
#include <memory>

class Lexer {
public:
    explicit Lexer(const std::string& fileName = "");
    Lexer(const Lexer&);
    Lexer(Lexer&&) noexcept;
    Token GetToken();
    void PrintLoggedTokens();
private:
    class FSM {
    private:
        struct State {
            uint8_t actions[256] = { 0 };
            void (*TS)(Token&) = nullptr; // TokenSet
        };
    public:
        FSM();
        ~FSM();
        State* states;
    };

    int ReadNextChar();
    std::string TokenIdToStr(int id);
    Token LogToken(Token &token);
    Token MakeToken();

    std::list<Token> loggedTokens;
    std::shared_ptr<std::istream> input;
    static const FSM fsm;
};
