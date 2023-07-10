#pragma once

#include <memory>
#include <stdio.h>
#include "Logger.hpp"
#include "Token.hpp"
#include "Lexer.hpp"
#include "CodeGenerator.hpp"

class Parser {
public:
    explicit Parser(std::unique_ptr<Lexer>);
    void ParseLoop();
private:
    class FSM {
    public:
        struct State {
            std::list<Token> expectedTokens = {};
            int next = 0;
            bool isReadNextToken = false;
            int toStack = 0;
            int error = -1; // if -1 then error can NOT be ignored;
            std::function<void(Token)> PushToken = nullptr;
            std::function<bool()> CG = nullptr;
        };
        FSM();
        ~FSM();
        State* states;
        CodeGenerator codeGen;
    };

    bool isCurrentTokenExpected();

    std::unique_ptr<Lexer> lexer;
    Token currentToken;
    FSM::State* currentState;
    std::stack<int> whereIsNext;
    FSM fsm;
};
