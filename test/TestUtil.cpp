#include "gtest/gtest.h"

// #define X

#ifdef X

#include "../src/impl/util.h"

TEST(UTIL, firstSet) {
    EXPECT_EQ(KParser::test_keych('?'), true);
    EXPECT_EQ(KParser::test_keych('a'), false);
}

#endif