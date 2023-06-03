#include "Parser.hpp"
#include <memory>

Parser::Parser(std::unique_ptr<Lexer> l) : lexer(std::move(l)) {}

void Parser::ParseLoop() {
    currentState = &fsm.states[2]; // starting from state 2
    currentToken = lexer->GetToken();

    do {
        fmt::print("Operating Token {} {}\n", currentToken.typeToStr(), currentToken.subtypeToStr()); // DEBUG

        if (!isCurrentTokenExpected()) {
            if (currentState->error > -1) {
                currentState = &fsm.states[currentState->error];
                continue;
            } else {
                fmt::print("Expected token not satisfied\n");
                return;
            }
        }

        if (currentState->PushToken != nullptr) {
            currentState->PushToken(currentToken);
        }

        if (currentState->CG != nullptr) {
            bool successCodeGen = currentState->CG();
            if (!successCodeGen) {
                fmt::print("Error in generating cocde\n");
                return;
            }
        }

        if (currentState->isReadNextToken) {
            currentToken = lexer->GetToken();
        }

        if (currentState->toStack > 0) {
            whereIsNext.push(currentState->toStack);
        }

        if (currentState->next != -1) {
            currentState = &fsm.states[currentState->next];
        } else {
            if (whereIsNext.empty()) {
                fmt::print("Stack(whereIsNext) is empty!!\n");
                return;
            }
            currentState = &fsm.states[whereIsNext.top()];
            whereIsNext.pop();
        }
    } while (currentState->next != 0);
}

bool Parser::isCurrentTokenExpected() {
    bool isExpectedFound = true;
    if (!currentState->expectedTokens.empty()) {
        isExpectedFound = std::any_of(
            currentState->expectedTokens.begin(),
            currentState->expectedTokens.end(),
            [&](const Token& t) {
                return t.type == currentToken.type
                    and t.subtype == currentToken.subtype;
        });
    }
    return isExpectedFound;
}

Parser::FSM::FSM() {
    states = new State[58];

    states[ 0] = { {},  0, false,  0,  -1 };
    states[ 1] = { {}, -1, false,  0,  -1 };
    states[ 2] = { {},  4, false,  3,  -1 };
    states[ 3] = { {},  0, false,  0,  -1 };
    states[ 3].expectedTokens.push_back({ Token::Eof, Token::None });

    states[ 4] = { {}, 12, false,  5,   1 };
    states[ 4].expectedTokens.push_back({ Token::Keyword, Token::Extern });
    states[ 4].expectedTokens.push_back({ Token::Keyword, Token::Def });
    states[ 4].expectedTokens.push_back({ Token::Operator, Token::Minus });
    states[ 4].expectedTokens.push_back({ Token::Identifier, Token::None });
    states[ 4].expectedTokens.push_back({ Token::Number, Token::None });
    states[ 4].expectedTokens.push_back({ Token::Separator, Token::LParen });

    states[ 5] = { {},  6,  true,  0,  -1 };
    states[ 6] = { {},  7, false,  0,  -1 };
    states[ 7] = { {},  4, false,  0,  -1 };
    states[ 8] = { {}, 11, false,  0,  -1 };
    states[ 9] = { {}, 13, false,  0,  -1 };
    states[10] = { {}, 25, false,  0,  -1 };
    states[11] = { {}, 12,  true,  0,  -1 };
    states[12] = { {}, 16, false,  0,  -1 };
    states[13] = { {}, 14,  true,  0,  -1 };
    states[14] = { {}, 16, false,  0,  -1 };
    states[15] = { {}, 25, false,  0,  -1 };
    states[16] = { {}, 17,  true,  0,  -1 };
    states[17] = { {}, 18,  true,  0,  -1 };
    states[18] = { {}, 20, false,  0,  -1 };
    states[19] = { {}, -1,  true,  0,  -1 };
    states[20] = { {}, 21,  true,  0,  -1 };
    states[21] = { {}, 22, false,  0,  -1 };
    states[22] = { {}, 23,  true,  0,  -1 };
    states[23] = { {}, 24,  true,  0,  -1 };
    states[24] = { {}, 24, false,  0,  -1 };
    states[25] = { {}, 26, false,  0,  -1 };
    states[26] = { {}, 31, false,  0,  -1 };
    states[27] = { {}, 28, false,  0,  -1 };
    states[28] = { {}, 45, false,  0,  -1 };
    states[29] = { {}, 31, false,  0,  -1 };
    states[30] = { {}, 28, false,  0,  -1 };
    states[31] = { {}, 36, false,  0,  -1 };
    states[32] = { {}, 33, false,  0,  -1 };
    states[33] = { {}, 47, false,  0,  -1 };
    states[34] = { {}, 36, false,  0,  -1 };
    states[35] = { {}, 33, false,  0,  -1 };
    states[36] = { {}, 39, false,  0,  -1 };
    states[37] = { {}, 38,  true,  0,  -1 };
    states[38] = { {}, 39, false,  0,  -1 };
    states[39] = { {}, -1,  true,  0,  -1 };
    states[40] = { {}, -1,  true,  0,  -1 };
    states[41] = { {}, 42,  true,  0,  -1 };
    states[42] = { {}, 25, false,  0,  -1 };
    states[43] = { {}, -1,  true,  0,  -1 };
    states[44] = { {}, 49, false,  0,  -1 };
    states[45] = { {}, -1,  true,  0,  -1 };
    states[46] = { {}, -1,  true,  0,  -1 };
    states[47] = { {}, -1,  true,  0,  -1 };
    states[48] = { {}, -1,  true,  0,  -1 };
    states[49] = { {}, 50,  true,  0,  -1 };
    states[50] = { {}, 51,  true,  0,  -1 };
    states[51] = { {}, 53, false,  0,  -1 };
    states[52] = { {}, -1,  true,  0,  -1 };
    states[53] = { {}, 25, false,  0,  -1 };
    states[54] = { {}, 55, false,  0,  -1 };
    states[55] = { {}, 56,  true,  0,  -1 };
    states[56] = { {}, 25, false,  0,  -1 };
    states[57] = { {}, 55, false,  0,  -1 };

    states[0].expectedTokens.push_back({ Token::Identifier, Token::None });
    states[0].CG = [this]() -> bool { return codeGen.GenNumber(); };
}

Parser::FSM::~FSM() {
    delete[] states;
}
