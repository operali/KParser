#include "gtest/gtest.h"
#include "../src/KParser.h"


TEST(Data, test) {
    {
        auto pParser = KParser::create();
        EXPECT_EQ(KParser::Parser::count, 1);
    }
    EXPECT_EQ(KParser::Parser::count, 0);
}

TEST(Data, test2) {
    EXPECT_EQ(KParser::Parser::count, 0);
    auto pParser = KParser::create();
    auto& p = *pParser;
    {
        std::string ret;
        auto n = p.all(
            p.str("abc"),
            p.str("123")->on([&](auto* m) {
                ret = *(m->get<std::string>());
                })
        );
        try {
            EXPECT_EQ(n->parse("abc123")!=nullptr, true);
            EXPECT_EQ(ret, "123");
        }
        catch (const std::exception& _) {
            EXPECT_EQ(true, false);
        }
    }
}


TEST(Data, test1) {
    /*auto p = KParser::Data();
    p.push<int>(3);
    p.push(std::string("123"));

    try {
        EXPECT_EQ(p.get<int>(0), 3);
        EXPECT_EQ(p.get<std::string>(1), "123");
        p.set<std::string>("abcde");
        EXPECT_EQ(p.get<std::string>(0), "abcde");

    }
    catch (const std::exception& ex) {
        EXPECT_EQ(true, false);
    }*/
}


TEST(TEST_PARSER, CONSTRUCTOR) {
    EXPECT_EQ(KParser::Parser::count, 0);
    auto pParser = KParser::create();
    auto& p = *pParser;
    auto r = p.any(
        p.str("abc"),
        p.str("123")
    );
    ASSERT_EQ(r->as<KParser::RuleAny>() != nullptr, true);
}

TEST(TEST_PARSER, EMPTY) {
    auto pParser = KParser::create();
    auto& p = *pParser;
    {
        auto noneRule = p.none();
        auto m = noneRule->match("", 0, 0);
        ASSERT_EQ(m->alter(), true);
        EXPECT_EQ(m->size(), 0);
        EXPECT_EQ(m->alter(), false);
    }
}

TEST(TEST_PARSER, STR) {
    
    auto pParser = KParser::create();
    auto& p = *pParser;
    auto r = p.str("1234");
    {
        auto m = r->match("1234", 4, 0);
        ASSERT_EQ(m->alter(), true);
        EXPECT_EQ(m->alter(), false);
        std::string v = m->str();
        EXPECT_EQ(v, "1234");
        EXPECT_EQ(m->size(), 4);
    }

    {
        EXPECT_EQ(r->parse("123"), nullptr);
    }

    {
        auto m = r->match("12345", 3, 0);
        EXPECT_EQ(m->alter(), false);
    }
}

TEST(TEST_PARSER, ANY) {
    auto pParser = KParser::create();
    auto& p = *pParser;
    {
        auto r = p.all(
            p.str("1234"),
            p.str("5678")
        );
        auto m = r->match("12345678", 8, 0);
        EXPECT_EQ(m->alter(), true);
        std::string v = m->str();
        EXPECT_EQ(v, "12345678");
        EXPECT_EQ(m->size(), 8);
        EXPECT_EQ(m->alter(), false);
    }
    {
        auto r = p.all();
        r->cons(
            p.str("1234"),
            p.str("5678")
        );
        auto m = r->match("12345678", 8, 0);
        EXPECT_EQ(m->alter(), true);
        std::string v = m->str();
        EXPECT_EQ(v, "12345678");
        EXPECT_EQ(m->size(), 8);
        EXPECT_EQ(m->alter(), false);
    }
}

TEST(TEST_PARSER, any_1) {
    auto pParser = KParser::create();
    auto& p = *pParser;
    {
        auto r = p.any(
            p.str("1234"),
            p.str("5678")
        );
        auto m = r->match("5678    ", 4, 0);
        EXPECT_EQ(m->alter(), true);
        std::string v = m->str();
        EXPECT_EQ(v, "5678");
        EXPECT_EQ(m->size(), 4);
        EXPECT_EQ(m->alter(), false);
    }
    {
        auto r = p.any();
        r->cons(
            p.str("1234"),
            p.str("5678")
        );
        auto m = r->match("5678    ", 4, 0);
        EXPECT_EQ(m->alter(), true);
        std::string v = m->str();
        EXPECT_EQ(v, "5678");
        EXPECT_EQ(m->size(), 4);
        EXPECT_EQ(m->alter(), false);
    }
}

TEST(TEST_PARSER, ALL) {
    auto pParser = KParser::create();
    auto& p = *pParser;
    auto k = p.any(
        p.str("1234"),
        p.str("5678")
    );
    auto r = p.all (
        k,  k
    );
    auto m = r->match("5678 1234", 9, 0);
    EXPECT_EQ(m->alter(), true);
    std::string v = m->str();
    EXPECT_EQ(v, "5678 1234");
    EXPECT_EQ(m->size(), 9);
    EXPECT_EQ(m->alter(), false);
}


TEST(TEST_PARSER, print) {
    auto pParser = KParser::create();
    auto& p = *pParser;
    {
        EXPECT_EQ(KParser::printRule(p.str("abc")), R"(Str(abc)
)");
    }
    {
        EXPECT_EQ(KParser::printRule(p.none()), R"(Empty
)");
    }
    {
        auto r = p.all(
            p.str("abc")
        );
        EXPECT_EQ(KParser::printRule(r), R"(All
  Str(abc)
)");
    }
    {

        auto r = p.all();
        r->cons(r);
        EXPECT_EQ(KParser::printRule(r), R"(All:0
  All:0
)");
    }
    {
        auto r = p.all();
        auto r1 = p.all();
        r->cons(r1);
        r1->cons(r1);
        EXPECT_EQ(KParser::printRule(r), R"(All
  All:0
    All:0
)");
    }
    {
        auto r = p.all();
        auto r1 = p.all();
        r->cons(r1);
        r1->cons(r);   
        EXPECT_EQ(KParser::printRule(r1), R"(All:0
  All
    All:0
)");
    }
    {
        auto r = p.str("abc");
        auto ks = p.many(r);
        auto s = KParser::printRule(ks);
        EXPECT_EQ(s,
            R"(Any:0
  All
    Str(abc)
    Any:0
  Empty
)");
    }
}

TEST(TEST_PARSER, MANY_SUB1) {
    auto pParser = KParser::create();
    auto& p = *pParser;

    auto k = p.any(
        p.str("1234"),
        p.none()
    );
    auto m = k->match("abcde", 5, 0);
    ASSERT_EQ(m->alter(), true);
    ASSERT_EQ(m->str(), "");
}

TEST(TEST_PARSER, MANY) {
    auto pParser = KParser::create();
    auto& p = *pParser;
    
    auto k = p.str("abc");
    auto ks = p.many(k);
    {
        auto m = ks->match("", 0, 0);
        EXPECT_EQ(m->alter(), true);
        std::string v = m->str();
        EXPECT_EQ(v, "");
    }
    {
        auto m = ks->match("abc", 3, 0);
        EXPECT_EQ(m->alter(), true);
        std::string v = m->str();
        EXPECT_EQ(v, "abc");
    }
    {
        auto m = ks->match("abcabc", 6, 0);
        EXPECT_EQ(m->alter(), true);
        std::string v = m->str();
        EXPECT_EQ(v, "abcabc");
    }
}

TEST(TEST_PARSER, MANY2) {
    auto pParser = KParser::create();
    auto& p = *pParser;
    
    auto k = p.any(
        p.str("abc"),
        p.str("123")
    );
    auto ks = p.many(k);
    {
        auto m = ks->match("abc", 3, 0);
        EXPECT_EQ(m->alter(), true);
        std::string v = m->str();
        EXPECT_EQ(v, "abc");
    }
    {
        auto m = ks->match("abc123", 6, 0);
        EXPECT_EQ(m->alter(), true);
        std::string v = m->str();
        EXPECT_EQ(v, "abc123");
    }
    {
        auto m = ks->match("abc 123abc abc 12312", 20, 0);
        EXPECT_EQ(m->alter(), true);
        std::string v = m->str();
        EXPECT_EQ(v, "abc 123abc abc 123");
    }
}

TEST(TEST_PARSER, MANY3) {
    auto pParser = KParser::create();
    auto& p = *pParser;
    
    auto k = p.any(
        p.str("abc"),
        p.str("123")
    );
    auto ks = p.many1(k);
    {
        auto m = ks->match("", 0, 0);
        EXPECT_EQ(m->alter(), false);
    }
    {
        auto m = ks->match("abc", 3, 0);
        EXPECT_EQ(m->alter(), true);
        std::string v = m->str();
        EXPECT_EQ(v, "abc");
    }
    {
        auto m = ks->match("abc123", 6, 0);
        EXPECT_EQ(m->alter(), true);
        std::string v = m->str();
        EXPECT_EQ(v, "abc123");
    }
    {
        auto m = ks->match("  abc 123abc abc 12312", 22, 0);
        EXPECT_EQ(m->alter(), true);
        std::string v = m->str();
        EXPECT_EQ(v, "abc 123abc abc 123");
    }
}

TEST(TEST_PARSER, PRED) {
    auto pParser = KParser::create();
    auto& p = *pParser;
    
    auto k = p.pred([](auto b, auto e, auto& val)->const char* {
        const char* c = b;
        int count = 0;
        while (c != e) {
            if (*c++ == 'd') {
                count++;
                if (count == 3) {
                    val = std::string(b, c);
                    return c;
                }
            }
        }
        return nullptr;
    });
    {
        auto m = k->match("abdddd", 6, 0);
        ASSERT_EQ(m->alter(), true);
        EXPECT_EQ(m->str(), "abddd");
        EXPECT_EQ(*m->get<std::string>(), "abddd");
        ASSERT_EQ(m->alter(), false);
    };
}


TEST(TEST_PARSER, OPTIONAL) {
    auto pParser = KParser::create();
    auto& p = *pParser;
    
    auto r = p.optional(
        p.str("abc")
    );
    {
        auto m = r->match("", 0, 0);
        ASSERT_EQ(m->alter(), true);
        EXPECT_EQ(m->str(), "");
    };
    {
        auto m = r->match("abc", 3, 0);
        ASSERT_EQ(m->alter(), true);
        EXPECT_EQ(m->str(), "abc");
    };
}

TEST(TEST_PARSER, until) {
    auto pParser = KParser::create();
    auto& p = *pParser;
    auto k = p.until(
        p.str("abc")
    );
    {
        auto m = k->match("12341234abc", 11, 0);
        ASSERT_EQ(m->alter(), true);
        EXPECT_EQ(m->str(), "12341234");
    }

    {
        auto m = k->match("12341234", 8, 0);
        ASSERT_EQ(m->alter(), false);
    };
}

TEST(TEST_PARSER, function) {
    auto pParser = KParser::create();
    auto& p = *pParser;

    auto keywordR = p.any(
        p.str("const"),
        p.str("static"),
        p.str("inline")
    );

    auto typeR = p.any(
        p.str("int"),
        p.str("float"),
        p.str("string"),
        p.str("int")
    );


    auto k = p.until(
        p.str("abc")
    );
    {
        auto m = k->match("12341234abc", 11, 0);
        ASSERT_EQ(m->alter(), true);
        EXPECT_EQ(m->str(), "12341234");
    }

    {
        auto m = k->match("12341234", 8, 0);
        ASSERT_EQ(m->alter(), false);
    };
}

TEST(REGEX, RE1) {
    auto pParser = KParser::create();
    auto& p = *pParser;
    
    {
        auto r = p.regex("[0-9]+");
        auto m = r->parse(" a12345bc");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "a12345");
        EXPECT_EQ(*m->get<std::string>(), "12345");
    }

    {
        auto r = p.regex("^[0-9]+");
        auto m = r->match("12345bc", 7, 0);
        auto res = m->alter();
        ASSERT_EQ(res, true);
        EXPECT_EQ(m->str(), "12345");
    }
}
TEST(REGEX, RE2) {
    auto pParser = KParser::create();
    auto& p = *pParser;
    
    {
        auto r = p.all(p.none(), p.regex("^[a-zA-Z_][a-zA-Z0-9_]*$"));
        auto* m = r->parse("0asdfasfd");
        ASSERT_EQ(m, nullptr);
    }
    {
        std::string v;
        auto r = p.all(
            p.none(), 
            p.regex("^[a-zA-Z_][a-zA-Z0-9_]*$")->on([&](auto* m) {
                v = *m->get<std::string>();
            })
        );
        auto* m = r->parse("  _1234_");
        ASSERT_EQ(m != nullptr, true);
        ASSERT_EQ(v, "_1234_");
        ASSERT_EQ(m->str(), "_1234_");
    }
}

TEST(COMPLICATE, T1) {
    EXPECT_EQ(KParser::Parser::count, 0);
    auto pParser = KParser::create();
    auto& p = *pParser;
    
    bool is_cons = false;
    bool is_static = false;
    bool is_inline = false;
    std::string retType;
    auto mod = p.any(
        p.str("const")->on([&](auto* m) {
            is_cons = true; 
            }),
        p.str("static")->on([&](auto* m) {
                is_static = true; 
            }),
        p.str("inline")->on([&](auto* m) {
                is_inline = true; 
            })
    );
    auto type = [&]() {
        return p.any(
            p.str("float"),
            p.str("int"),
            p.str("bool"),
            p.str("string")
        );
    };

    struct argType {
        std::string type;
        std::string name;
    };
    std::vector<argType> args;

    argType tmpType;
    auto startArg = [&](auto* m) {
        tmpType.type = m->str();
    };

    auto stopArg = [&](auto* m) {
        tmpType.name = m->str();
        args.push_back(tmpType);
    };

    auto arg = p.all(
        type()->on(startArg),
        p.identifier()->on(stopArg)
    );

    auto func = p.all(
        p.many(mod),
        type()->on([&](auto* m) {retType = m->str(); }),
        p.identifier(),
        p.str("("),
        p.list(arg, p.str(",")),
        p.str(")")
    );
    std::string t = "  const float myfun(int a, string b); ";
    func->parse(t);
    int i = 0;
}