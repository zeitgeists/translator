#include <gtest/gtest.h>
// #include "Lexer.hpp"

#include <fstream>
#include <iostream>

TEST(LexerTests, L) {
    char ch;
    std::fstream fin("/home/bluten/Projects/translator/tests/test_input.txt", std::fstream::in);
    EXPECT_EQ(fin.is_open(), true);

    // Lexer::ReadNextChar();
    fin.get(ch);
    EXPECT_EQ(ch, '5');
    fin.get(ch);
    EXPECT_EQ(ch, '+');
    fin.get(ch);
    EXPECT_EQ(ch, '9');
}
