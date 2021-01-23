#include "gtest/gtest.h"
#include "../src/KParser.h"
#include "../src/impl/dsl.h"

// #define X
#ifndef X
TEST(DSL_BASIC, str) {
    {
        DSLContext ctx;
        ctx.parse(R"(
a = b;
b = c d;
d = e | a b;
)");
    }
}

#endif