#include "Parser.hpp"
#include <memory>

Parser::Parser(std::unique_ptr<Lexer> l) : lexer(std::move(l)) {}

void Parser::ParseLoop() {
    lexer->GetToken();
}

Parser::FSM::FSM() {
    states = new State[57];

    states[ 0] = { {},  0, false,  0, false, false,  -1, nullptr };
    states[ 1] = { {}, -1, false,  0, false, false,  -1, nullptr };
    states[ 2] = { {},  4, false,  3, false, false,  -1, nullptr };
    states[ 3] = { {},  0, false,  0, false, false,  -1, nullptr };
    states[ 4] = { {}, 12, false,  5, false, false,  -1, nullptr };
    states[ 5] = { {},  6,  true,  0, false, false,  -1, nullptr };
    states[ 6] = { {},  7, false,  0, false, false,  -1, nullptr };
    states[ 7] = { {},  4, false,  0, false, false,  -1, nullptr };
    states[ 8] = { {}, 11, false,  0, false, false,  -1, nullptr };
    states[ 9] = { {}, 13, false,  0, false, false,  -1, nullptr };
    states[10] = { {}, 25, false,  0, false, false,  -1, nullptr };
    states[11] = { {}, 12,  true,  0, false, false,  -1, nullptr };
    states[12] = { {}, 16, false,  0, false, false,  -1, nullptr };
    states[13] = { {}, 14,  true,  0, false, false,  -1, nullptr };
    states[14] = { {}, 16, false,  0, false, false,  -1, nullptr };
    states[15] = { {}, 25, false,  0, false, false,  -1, nullptr };
    states[16] = { {}, 17,  true,  0, false, false,  -1, nullptr };
    states[17] = { {}, 18,  true,  0, false, false,  -1, nullptr };
    states[18] = { {}, 20, false,  0, false, false,  -1, nullptr };
    states[19] = { {}, -1,  true,  0, false, false,  -1, nullptr };
    states[20] = { {}, 21,  true,  0, false, false,  -1, nullptr };
    states[21] = { {}, 22, false,  0, false, false,  -1, nullptr };
    states[22] = { {}, 23,  true,  0, false, false,  -1, nullptr };
    states[23] = { {}, 24,  true,  0, false, false,  -1, nullptr };
    states[24] = { {}, 24, false,  0, false, false,  -1, nullptr };
    states[25] = { {}, 26, false,  0, false, false,  -1, nullptr };
    states[26] = { {}, 31, false,  0, false, false,  -1, nullptr };
    states[27] = { {}, 28, false,  0, false, false,  -1, nullptr };
    states[28] = { {}, 45, false,  0, false, false,  -1, nullptr };
    states[29] = { {}, 31, false,  0, false, false,  -1, nullptr };
    states[30] = { {}, 28, false,  0, false, false,  -1, nullptr };
    states[31] = { {}, 36, false,  0, false, false,  -1, nullptr };
    states[32] = { {}, 33, false,  0, false, false,  -1, nullptr };
    states[33] = { {}, 47, false,  0, false, false,  -1, nullptr };
    states[34] = { {}, 36, false,  0, false, false,  -1, nullptr };
    states[35] = { {}, 33, false,  0, false, false,  -1, nullptr };
    states[36] = { {}, 39, false,  0, false, false,  -1, nullptr };
    states[37] = { {}, 38,  true,  0, false, false,  -1, nullptr };
    states[38] = { {}, 39, false,  0, false, false,  -1, nullptr };
    states[39] = { {}, -1,  true,  0, false, false,  -1, nullptr };
    states[40] = { {}, -1,  true,  0, false, false,  -1, nullptr };
    states[41] = { {}, 42,  true,  0, false, false,  -1, nullptr };
    states[42] = { {}, 25, false,  0, false, false,  -1, nullptr };
    states[43] = { {}, -1,  true,  0, false, false,  -1, nullptr };
    states[44] = { {}, 49, false,  0, false, false,  -1, nullptr };
    states[45] = { {}, -1,  true,  0, false, false,  -1, nullptr };
    states[46] = { {}, -1,  true,  0, false, false,  -1, nullptr };
    states[47] = { {}, -1,  true,  0, false, false,  -1, nullptr };
    states[48] = { {}, -1,  true,  0, false, false,  -1, nullptr };
    states[49] = { {}, 50,  true,  0, false, false,  -1, nullptr };
    states[50] = { {}, 51,  true,  0, false, false,  -1, nullptr };
    states[51] = { {}, 53, false,  0, false, false,  -1, nullptr };
    states[52] = { {}, -1,  true,  0, false, false,  -1, nullptr };
    states[53] = { {}, 25, false,  0, false, false,  -1, nullptr };
    states[54] = { {}, 55, false,  0, false, false,  -1, nullptr };
    states[55] = { {}, 56,  true,  0, false, false,  -1, nullptr };
    states[56] = { {}, 25, false,  0, false, false,  -1, nullptr };
    states[57] = { {}, 55, false,  0, false, false,  -1, nullptr };

    states[0].expectedTokens.push_back({ Token::Identifier, Token::None });
}
