#include "Parser.hpp"
#include <memory>

Parser::Parser(std::unique_ptr<Lexer> l) : lexer(std::move(l)) {}

void Parser::ParseLoop() {
    currentState = &fsm.states[2]; // starting from state 2
    currentToken = lexer->GetToken();

    do {
        fmt::print("State: {} ", static_cast<int>(currentState - fsm.states));
        fmt::print("Operating Token [{}, {}, '{}']\n",
            currentToken.typeToStr(), currentToken.subtypeToStr(), currentToken.str); // DEBUG

        if (!isCurrentTokenExpected()) {
            if (currentState->error > -1) {
                currentState = &fsm.states[currentState->error];
                continue;
            } else {
                // DEBUG printing
                fmt::print("Expected token not satisfied. currentToken = [{}, {}, '{}']\n",
                    currentToken.typeToStr(), currentToken.subtypeToStr(), currentToken.str);
                for (const Token &t : currentState->expectedTokens) {
                fmt::print("Expected token not satisfied. currentToken = [{}, {}]\n",
                        t.typeToStr(), t.subtypeToStr());
                }
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

    fsm.codeGen.PrintGeneratedCode();
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
    states = new State[66];

    states[ 0] = { {},  0, false,  0,  -1 };
    states[ 1] = { {}, -1, false,  0,  -1 };
    states[ 2] = { {},  4, false,  3,  -1 };
    states[ 3] = { {},  0, false,  0,  -1 };
    states[ 4] = { {},  8, false,  5,   1 };
    states[ 5] = { {},  6,  true,  0,  -1 };
    states[ 6] = { {},  7, false,  0,  -1 };
    states[ 7] = { {},  4, false,  0,   1 };
    states[ 8] = { {},  9,  true,  0,  10 };
    states[ 9] = { {}, 13, false,  0,  -1 };
    states[10] = { {}, 11,  true,  0,  12 };
    states[11] = { {}, 14, false,  0,  -1 };
    states[12] = { {}, 25, false, 58,  -1 };
    states[13] = { {}, 16, false, 59,  -1 };
    states[14] = { {}, 16, false, 15,  -1 };
    states[15] = { {}, 25, false, 60,  -1 };
    states[16] = { {}, 17,  true,  0,  -1 };
    states[17] = { {}, 18,  true,  0,  -1 };
    states[18] = { {}, 20, false, 19,  -1 };
    states[19] = { {}, -1,  true,  0,  -1 };
    states[20] = { {}, 21,  true,  0,   1 };
    states[21] = { {}, 22, false,  0,  -1 };
    states[22] = { {}, 23,  true,  0,   1 };
    states[23] = { {}, 24,  true,  0,  -1 };
    states[24] = { {}, 22, false,  0,  -1 };
    states[25] = { {}, 26, false,  0,  -1 };
    states[26] = { {}, 31, false, 27,  -1 };
    states[27] = { {}, 28, false,  0,  -1 };
    states[28] = { {}, 49, false, 29,   1 };
    states[29] = { {}, 31, false, 61,  -1 };
    states[30] = { {}, 28, false,  0,  -1 };
    states[31] = { {}, 36, false, 32,  -1 };
    states[32] = { {}, 33, false,  0,  -1 };
    states[33] = { {}, 51, false, 34,   1 };
    states[34] = { {}, 36, false, 62,  -1 };
    states[35] = { {}, 33, false,  0,  -1 };
    states[36] = { {}, 39, false,  0,  37 };
    states[37] = { {}, 38,  true,  0,  -1 };
    states[38] = { {}, 39, false, 63,  -1 };
    states[39] = { {}, 40,  true,  0,  41 };
    states[40] = { {}, 45, false,  0,  -1 };
    states[41] = { {}, -1,  true,  0,  42 };
    states[42] = { {}, 43,  true,  0,  -1 };
    states[43] = { {}, 25, false, 44,  -1 };
    states[44] = { {}, -1,  true,  0,  -1 };
    states[45] = { {}, 46,  true,  0,  48 };
    states[46] = { {}, 53, false, 47,  -1 };
    states[47] = { {}, -1,  true,  0,  -1 };
    states[48] = { {}, -1, false,  0,  -1 };
    states[49] = { {}, -1,  true,  0,  50 };
    states[50] = { {}, -1,  true,  0,  51 };
    states[51] = { {}, -1,  true,  0,  52 };
    states[52] = { {}, -1,  true,  0,  -1 };
    states[53] = { {}, 25, false, 64,   1 };
    states[54] = { {}, 55, false,  0,  -1 };
    states[55] = { {}, 56,  true,  0,   1 };
    states[56] = { {}, 25, false, 65,  -1 };
    states[57] = { {}, 55, false,  0,  -1 };
    states[58] = { {}, -1, false,  0,  -1 };
    states[59] = { {}, -1, false,  0,  -1 };
    states[60] = { {}, -1, false,  0,  -1 };
    states[61] = { {}, 30, false,  0,  -1 };
    states[62] = { {}, 35, false,  0,  -1 };
    states[63] = { {}, -1, false,  0,  -1 };
    states[64] = { {}, 54, false,  0,  -1 };
    states[65] = { {}, 57, false,  0,  -1 };

    states[ 3].expectedTokens.push_back({ Token::Eof, Token::None });
    states[ 4].expectedTokens.push_back({ Token::Keyword, Token::Extern });
    states[ 4].expectedTokens.push_back({ Token::Keyword, Token::Def });
    states[ 4].expectedTokens.push_back({ Token::Operator, Token::Minus });
    states[ 4].expectedTokens.push_back({ Token::Identifier, Token::None });
    states[ 4].expectedTokens.push_back({ Token::Number, Token::None });
    states[ 4].expectedTokens.push_back({ Token::Separator, Token::LParen });
    states[ 5].expectedTokens.push_back({ Token::Separator, Token::Semicolon });
    states[ 7].expectedTokens.push_back({ Token::Keyword, Token::Extern });
    states[ 7].expectedTokens.push_back({ Token::Keyword, Token::Def });
    states[ 7].expectedTokens.push_back({ Token::Operator, Token::Minus });
    states[ 7].expectedTokens.push_back({ Token::Identifier, Token::None });
    states[ 7].expectedTokens.push_back({ Token::Number, Token::None });
    states[ 7].expectedTokens.push_back({ Token::Separator, Token::LParen });
    states[ 8].expectedTokens.push_back({ Token::Keyword, Token::Extern });
    states[10].expectedTokens.push_back({ Token::Keyword, Token::Def });
    states[16].expectedTokens.push_back({ Token::Identifier, Token::None });
    states[17].expectedTokens.push_back({ Token::Separator, Token::LParen });
    states[19].expectedTokens.push_back({ Token::Separator, Token::RParen });
    states[20].expectedTokens.push_back({ Token::Identifier, Token::None });
    states[22].expectedTokens.push_back({ Token::Separator, Token::Comma });
    states[23].expectedTokens.push_back({ Token::Identifier, Token::None });
    states[28].expectedTokens.push_back({ Token::Operator, Token::Plus });
    states[28].expectedTokens.push_back({ Token::Operator, Token::Minus });
    states[33].expectedTokens.push_back({ Token::Operator, Token::Multiply });
    states[33].expectedTokens.push_back({ Token::Operator, Token::Divide });
    states[36].expectedTokens.push_back({ Token::Identifier, Token::None });
    states[36].expectedTokens.push_back({ Token::Number, Token::None });
    states[36].expectedTokens.push_back({ Token::Separator, Token::LParen });
    states[37].expectedTokens.push_back({ Token::Operator, Token::Minus });
    states[39].expectedTokens.push_back({ Token::Identifier, Token::None });
    states[41].expectedTokens.push_back({ Token::Number, Token::None });
    states[42].expectedTokens.push_back({ Token::Separator, Token::LParen });
    states[44].expectedTokens.push_back({ Token::Separator, Token::RParen });
    states[45].expectedTokens.push_back({ Token::Separator, Token::LParen });
    states[47].expectedTokens.push_back({ Token::Separator, Token::RParen });
    states[49].expectedTokens.push_back({ Token::Operator, Token::Plus });
    states[50].expectedTokens.push_back({ Token::Operator, Token::Minus });
    states[51].expectedTokens.push_back({ Token::Operator, Token::Multiply });
    states[52].expectedTokens.push_back({ Token::Operator, Token::Divide });
    states[53].expectedTokens.push_back({ Token::Identifier, Token::None });
    states[53].expectedTokens.push_back({ Token::Number, Token::None });
    states[53].expectedTokens.push_back({ Token::Separator, Token::LParen });
    states[53].expectedTokens.push_back({ Token::Operator, Token::Minus });
    states[55].expectedTokens.push_back({ Token::Separator, Token::Comma });

    states[16].PushToken = [this](Token t) { codeGen.PushStr(t); };
    states[20].PushToken = [this](Token t) { codeGen.PushParam(t); };
    states[23].PushToken = [this](Token t) { codeGen.PushParam(t); };
    states[37].PushToken = [this](Token t) { codeGen.PushOperator(t); };
    states[39].PushToken = [this](Token t) { codeGen.PushStr(t); };
    states[41].PushToken = [this](Token t) { codeGen.PushStr(t); };
    states[49].PushToken = [this](Token t) { codeGen.PushOperator(t); };
    states[50].PushToken = [this](Token t) { codeGen.PushOperator(t); };
    states[51].PushToken = [this](Token t) { codeGen.PushOperator(t); };
    states[52].PushToken = [this](Token t) { codeGen.PushOperator(t); };
    states[64].PushToken = [this](Token t) { codeGen.PushArg(t); };
    states[65].PushToken = [this](Token t) { codeGen.PushArg(t); };

    states[58].CG = [this]() -> bool { return codeGen.GenAnonFunction(); };
    states[59].CG = [this]() -> bool { return codeGen.GenExtern(); };
    states[60].CG = [this]() -> bool { return codeGen.GenDef(); };
    states[61].CG = [this]() -> bool { return codeGen.GenOperator(); };
    states[62].CG = [this]() -> bool { return codeGen.GenOperator(); };
    states[63].CG = [this]() -> bool { return codeGen.GenNegation(); };
    states[41].CG = [this]() -> bool { return codeGen.GenNumber(); };
    states[47].CG = [this]() -> bool { return codeGen.GenCall(); };
    states[48].CG = [this]() -> bool { return codeGen.GenVariable(); };
}

Parser::FSM::~FSM() {
    delete[] states;
}
