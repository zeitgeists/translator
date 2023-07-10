#include "Lexer.hpp"

const Lexer::FSM Lexer::fsm;

Lexer::Lexer(const std::string& fileName) {
    if (!fileName.empty()) {
        std::ifstream* fileStream = new std::ifstream(fileName);
        if (!fileStream->is_open()) {
            // TODO: use logging
            fmt::print("Failed to open file: {}\n", fileName);
            delete fileStream;
            input.reset(&std::cin, [](void*) {});
        } else {
            input.reset(fileStream, [](void* ptr) {
                delete static_cast<std::ifstream*>(ptr);
            });
        }
    } else {
        input.reset(&std::cin, [](void*) {});
    }
}

// Copy constructor
Lexer::Lexer(const Lexer& arg)
    : loggedTokens(arg.loggedTokens),
    input(arg.input) {
}

// Move constructor
Lexer::Lexer(Lexer&& arg) noexcept
    : loggedTokens(std::move(arg.loggedTokens)),
      input(std::move(arg.input)) {
}

int Lexer::ReadNextChar() {
    return input->get();
}

void Lexer::PrintLoggedTokens() {
    Token t;
    fmt::print("{:^13}|{:^13}|{:^13}\n", "type", "subtype", "str");
    while (!loggedTokens.empty()) {
        t = loggedTokens.front();
        loggedTokens.pop_front();
        fmt::print("{:^13}|{:^13}|{:^13}\n",
            t.typeToStr(), t.subtypeToStr(), t.str);
    }
}

Token Lexer::GetToken() {
    Token token = MakeToken();
    loggedTokens.push_back(token);
    return token;
}

Token Lexer::MakeToken() {
    Token t;
    static int currentChar = ' ';
    std::string buf;

    do {
        if (currentChar == EOF) {
            TokenSet::Eof(t);
            return t;
        }
        buf = "";
        unsigned int currentState = 0;
        do {
            currentState = fsm.states[currentState].actions[currentChar];
            buf += currentChar;
            currentChar = Lexer::ReadNextChar();
        } while ((currentState != 0)
                and (fsm.states[currentState].actions[currentChar] != 0));

        if (fsm.states[currentState].TS != nullptr) {
            fsm.states[currentState].TS(t);
        } else {
            // Log this
            fmt::print("TokenSet reference is NULL!!! Lexer FSM table WRONG:\n");
            fmt::print("\tcurrentState = {}\n", currentState);
            fmt::print("\tcurrentChar = {}\n", currentChar);
            TokenSet::Illegal(t);
        }
    } while ((t.type == Token::Whitespace) or (t.type == Token::Comment));

    t.str = buf;

    return t;
}

Lexer::FSM::FSM() {
    enum Indexes {
        Start, // NOT ATUAL INDEX! uses only to start enum from 1
        IllegalIdx,
        WhitespaceIdx,
        LineCommentIdx,
        IdentifierIdx,
        NumberIdx,
        NumberWithoutDotIdx,
        PlusIdx,
        MinusIdx,
        MultiplyIdx,
        DivideIdx,
        CommaIdx,
        SemicolonIdx,
        LParenIdx,
        RParenIdx,
        LCurlyIdx,
        RCurlyIdx,

        // Keyword section:
        // extern
        K_e,
        K_e_x,
        K_ex_t,
        K_ext_e,
        K_exte_r,
        K_exter_n,
        // def
        K_d,
        K_d_e,
        K_de_f,

        Amount // NOT ATUAL INDEX! uses only to count amount of indexes
    };

    constexpr int MaxIdx = static_cast<int>(Amount);
    constexpr int KeywordStartIdx = static_cast<int>(K_e);

    states = new State[MaxIdx + 1];

    // Illegal
    for (int c = 0; c <= 255; c++) {
        states[0].actions[(uint8_t)c] = IllegalIdx;
    }
    states[IllegalIdx].TS = TokenSet::Illegal;

    // Whitespace
    states[0].actions[(uint8_t)' '] = WhitespaceIdx;
    states[0].actions[(uint8_t)'\n'] = WhitespaceIdx;
    states[0].actions[(uint8_t)'\r'] = WhitespaceIdx;
    states[0].actions[(uint8_t)'\t'] = WhitespaceIdx;
    states[WhitespaceIdx].actions[(uint8_t)' '] = WhitespaceIdx;
    states[WhitespaceIdx].actions[(uint8_t)'\n'] = WhitespaceIdx;
    states[WhitespaceIdx].actions[(uint8_t)'\r'] = WhitespaceIdx;
    states[WhitespaceIdx].actions[(uint8_t)'\t'] = WhitespaceIdx;
    states[WhitespaceIdx].TS = TokenSet::Whitespace;

    // Comment
    states[0].actions[(uint8_t)'#'] = LineCommentIdx;
    for (int c = 0; c <= 255; c++) {
        states[LineCommentIdx].actions[(uint8_t)c] = LineCommentIdx;
    }
    states[LineCommentIdx].actions[(uint8_t)'\n'] = 0;
    states[LineCommentIdx].actions[(uint8_t)'\r'] = 0;
    states[LineCommentIdx].TS = TokenSet::LineComment;

    // Identifier
    for (char c = 'a'; c <= 'z'; c++) {
        states[0].actions[(uint8_t)c] = IdentifierIdx;
        states[IdentifierIdx].actions[(uint8_t)c] = IdentifierIdx;
        for (int s = KeywordStartIdx; s <= MaxIdx; s++) {
            states[s].actions[(uint8_t)c] = IdentifierIdx;
            states[s].TS = TokenSet::Identifier;
        }
    }
    for (char c = 'A'; c <= 'Z'; c++) {
        states[0].actions[(uint8_t)c] = IdentifierIdx;
        states[IdentifierIdx].actions[(uint8_t)c] = IdentifierIdx;
        for (int s = KeywordStartIdx; s <= MaxIdx; s++) {
            states[s].actions[(uint8_t)c] = IdentifierIdx;
            states[s].TS = TokenSet::Identifier;
        }
    }
    for (char c = '0'; c <= '9'; c++) {
        states[0].actions[(uint8_t)c] = IdentifierIdx;
        states[IdentifierIdx].actions[(uint8_t)c] = IdentifierIdx;
        for (int s = KeywordStartIdx; s <= MaxIdx; s++) {
            states[s].actions[(uint8_t)c] = IdentifierIdx;
            states[s].TS = TokenSet::Identifier;
        }
    }
    states[IdentifierIdx].TS = TokenSet::Identifier;

    // Number
    for (char i = '0'; i <= '9'; i++) {
        states[0].actions[(uint8_t)i] = NumberIdx;
        states[NumberIdx].actions[(uint8_t)i] = NumberIdx;
    }
    states[NumberIdx].actions[(uint8_t)'.'] = NumberWithoutDotIdx;
    states[NumberIdx].TS = TokenSet::Number;

    for (char i = '0'; i <= '9'; i++) {
        states[NumberWithoutDotIdx]
            .actions[(uint8_t)i] = NumberWithoutDotIdx;
    }
    states[NumberWithoutDotIdx].TS = TokenSet::Number;

    // Operator
    states[0].actions[(uint8_t)'+'] = PlusIdx;
    states[PlusIdx].TS = TokenSet::Plus;

    states[0].actions[(uint8_t)'-'] = MinusIdx;
    states[MinusIdx].TS = TokenSet::Minus;

    states[0].actions[(uint8_t)'*'] = MultiplyIdx;
    states[MultiplyIdx].TS = TokenSet::Multiply;

    states[0].actions[(uint8_t)'/'] = DivideIdx;
    states[DivideIdx].TS = TokenSet::Divide;

    // Separator
    states[0].actions[(uint8_t)','] = CommaIdx;
    states[CommaIdx].TS = TokenSet::Comma;

    states[0].actions[(uint8_t)';'] = SemicolonIdx;
    states[SemicolonIdx].TS = TokenSet::Semicolon;

    states[0].actions[(uint8_t)'('] = LParenIdx;
    states[LParenIdx].TS = TokenSet::LParen;

    states[0].actions[(uint8_t)')'] = RParenIdx;
    states[RParenIdx].TS = TokenSet::RParen;

    states[0].actions[(uint8_t)'{'] = LCurlyIdx;
    states[LCurlyIdx].TS = TokenSet::LCurly;

    states[0].actions[(uint8_t)'}'] = RCurlyIdx;
    states[RCurlyIdx].TS = TokenSet::RCurly;

    // Keyword
    //extern
    states[0].actions[(uint8_t)'e'] = K_e;
    states[K_e].actions[(uint8_t)'x'] = K_e_x;
    states[K_e_x].actions[(uint8_t)'t'] = K_ex_t;
    states[K_ex_t].actions[(uint8_t)'e'] = K_ext_e;
    states[K_ext_e].actions[(uint8_t)'r'] = K_exte_r;
    states[K_exte_r].actions[(uint8_t)'n'] = K_exter_n;
    states[K_exter_n].TS = TokenSet::Extern;
    //def
    states[0].actions[(uint8_t)'d'] = K_d;
    states[K_d].actions[(uint8_t)'e'] = K_d_e;
    states[K_d_e].actions[(uint8_t)'f'] = K_de_f;
    states[K_de_f].TS = TokenSet::Def;
}

Lexer::FSM::~FSM() {
    delete[] states;
}
