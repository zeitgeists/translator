#include <gtest/gtest.h>
#include "Parser.hpp"

TEST(ParserTests, T) {
    EXPECT_STRNE("hello", "world");
    EXPECT_EQ(7 * 6, 42);
}
