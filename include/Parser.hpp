#pragma once

#include <memory>
#include <stdio.h>
#include "Logger.hpp"
#include "Token.hpp"
#include "Lexer.hpp"
#include "CodeGenerator.hpp"
// #include "AST.hpp"

class Parser {
public:
    explicit Parser(std::unique_ptr<Lexer>);
    void ParseLoop();
private:
    class FSM {
    private:
        struct State {
            std::list<Token> expectedTokens = {};
            int next = 0;
            bool isReadNextToken = false;
            int toStack = 0;
            bool pushTerm = false;
            bool pushOperator = false;
            int error = -1; // if -1 then error can NOT be ignored;
            void (*CG)() = nullptr; // CodeGen
        };
    public:
        FSM();
        ~FSM();
        State* states;
    };

    std::unique_ptr<Lexer> lexer;
    Token currentToken;
    static const FSM fsm;
    CodeGenerator codeGen;
};
