#include "gtest/gtest.h"
#include "../src/KParser.h"
#include "../src/impl/rule.h"
#include <sstream>
#include <Windows.h>
#include <exception>

//TEST(Data, test) {
//    class MyParser : public KParser::Parser {
//    };
//    {
//        MyParser parser;
//        EXPECT_EQ(KParser::KObject::count, 1);
//    }
//    EXPECT_EQ(KParser::KObject::count, 0);
//}
//
//TEST(Data, test2) {
//
//    // EXPECT_EQ(KParser::KObject::count, 0);
//    {
//        struct MyParser : public KParser::Parser {
//            std::string val;
//            KParser::Rule* ruleOf() {
//                return any(
//                    str("abc"),
//                    str("123")->on([this](auto* m) {
//                        std::cout << "hello" << std::endl;
//                        this->val = m->str();
//                     })
//                );
//            }
//        };
//
//        {
//            MyParser parser;
//            try {
//                auto m = parser.ruleOf()->match("123");
//                EXPECT_EQ( m != nullptr, true);
//                EXPECT_EQ(parser.val, "123");
//                
//            }
//            catch (const std::exception& _) {
//                EXPECT_EQ(true, false);
//            }
//        }
//    }
//
//    EXPECT_EQ(KParser::KObject::count, 0);
//}
////
////
//TEST(TEST_PARSER, EMPTY) {
//    // EXPECT_EQ(KParser::KObject::count, 0);
//    {
//        KParser::Parser p;
//        {
//            auto noneRule = (KParser::RuleNode*)p.none();
//            auto m = std::unique_ptr<KParser::MatchR>(noneRule->match("", 0, 0));
//            ASSERT_EQ(m->alter(), true);
//            EXPECT_EQ(m->size(), 0);
//            EXPECT_EQ(m->alter(), false);
//        }
//    }
//    // EXPECT_EQ(KParser::KObject::count, 0);
//}
////
////
//TEST(TEST_PARSER, STR) {
//    
//    // EXPECT_EQ(KParser::KObject::count, 0);
//    {
//        KParser::Parser p;
//        
//        auto r = p.str("1234");
//        {
//            auto m = r->match("1234");
//            std::string v = m->str();
//            EXPECT_EQ(v, "1234");
//            EXPECT_EQ(m->alter(), false);
//        }
//
//        {
//            p.reset();
//            EXPECT_EQ(r->match("123"), nullptr);
//        }
//
//        {
//            p.reset();
//            auto m = r->match("12345");
//            EXPECT_EQ(m != nullptr, true);
//        }
//    }
//    // EXPECT_EQ(KParser::KObject::count, 0);
//}
//
//
//
TEST(TEST_PARSER, ANY) {
    EXPECT_EQ(KParser::KObject::count, 0);
    {
        KParser::Parser p;
        {
            auto r = p.all(
                p.str("1234"),
                p.str("5678")
            );
            auto m = r->match("12345678");
            ASSERT_EQ(m != nullptr, true);
            std::string v = m->str();
            EXPECT_EQ(v, "12345678");
            EXPECT_EQ(m->alter(), false);
        }
        {
            auto r = p.all(
                p.str("1234"),
                p.str("5678")
            );
            
            auto m = r->match("12345678");
            ASSERT_EQ(m != nullptr, true);
            std::string v = m->str();
            EXPECT_EQ(v, "12345678");
            EXPECT_EQ(m->alter(), false);
        }
    }
    auto left = KParser::KObject::all;
    EXPECT_EQ(KParser::KObject::count, 0);
}

TEST(TEST_PARSER, any_1) {
    // EXPECT_EQ(KParser::KObject::count, 0);
    {
        KParser::Parser p;
        {
            auto r = (KParser::RuleNode*)p.any(
                p.str("1234"),
                p.str("5678")
            );
            auto m = r->match("5678    ", 4, 0);
            EXPECT_EQ(m->alter(), true);
            std::string v = m->str();
            EXPECT_EQ(v, "5678");
            EXPECT_EQ(m->alter(), false);
            delete m;
        }
        {
            auto r = (KParser::RuleNode*)p.any(
                p.str("1234"),
                p.str("5678")
            );
            
            auto m = r->match("5678    ", 4, 0);
            EXPECT_EQ(m->alter(), true);
            std::string v = m->str();
            EXPECT_EQ(v, "5678");
            EXPECT_EQ(m->size(), 4);
            EXPECT_EQ(m->alter(), false);
            delete m;
        }
    }
    EXPECT_EQ(KParser::KObject::count, 0);
}

TEST(TEST_PARSER, ALL) {
    EXPECT_EQ(KParser::KObject::count, 0);
    {
        KParser::Parser p;
        auto k = (KParser::RuleNode*)p.any(
            p.str("1234"),
            p.str("5678")
        );
        auto r = (KParser::RuleNode*)p.all(
            k, k
        );
        auto m = r->match("5678 1234", 9, 0);
        EXPECT_EQ(m->alter(), true);
        std::string v = m->str();
        EXPECT_EQ(v, "5678 1234");
        EXPECT_EQ(m->size(), 9);
        EXPECT_EQ(m->alter(), false);
        delete m;
    }
    EXPECT_EQ(KParser::KObject::count, 0);
}


TEST(TEST_PARSER, print) {
    KParser::Parser p;
    {
        EXPECT_EQ(p.str("abc")->toString(), R"(Str(abc)
)");
    }
    {
        EXPECT_EQ(p.none()->toString(), R"(Empty
)");
    }
    {
        auto r = p.all(
            p.str("abc")
        );
        EXPECT_EQ(r->toString(), R"(All
  Str(abc)
)");
    }
    {
        auto r = p.all();
        r->add(r);
        EXPECT_EQ(r->toString(), R"(All:0
  All:0
)");
    }
    {
        auto r = p.all();
        auto r1 = p.all();
        r->add(r1);
        r1->add(r1);
        EXPECT_EQ(r->toString(), R"(All
  All:0
    All:0
)");
    }
    {
        auto r = p.all();
        auto r1 = p.all();
        r->appendChild(r1);
        r1->appendChild(r);
        EXPECT_EQ(r1->toString(), R"(All:0
  All
    All:0
)");
    }
    {
        auto r = p.str("abc");
        auto ks = p.many(r);
        auto s = ks->toString();
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
    // EXPECT_EQ(KParser::KObject::count, 0);
    {
        KParser::Parser p;
        auto k = (KParser::RuleNode*)p.any(
            p.str("1234"),
            p.none()
        );
        auto m = k->match("abcde", 5, 0);
        ASSERT_EQ(m->alter(), true);
        ASSERT_EQ(m->str(), "");
        delete m;
    }
    // EXPECT_EQ(KParser::KObject::count, 0);
}

TEST(TEST_PARSER, MANY) {
    // EXPECT_EQ(KParser::KObject::count, 0);
    {
        KParser::Parser p;
        
        auto k = p.str("abc");
        auto ks = p.many(k);
        {
            auto m = ks->match("");
            EXPECT_EQ(m!=nullptr, true);
            std::string v = m->str();
            EXPECT_EQ(v, "");
        }
        {
            auto m = ks->match("abc");
            ASSERT_EQ(m != nullptr, true);
            std::string v = m->str();
            EXPECT_EQ(v, "abc");
        }
        {
            auto m = ks->match("abcabc");
            ASSERT_EQ(m != nullptr, true);
            std::string v = m->str();
            EXPECT_EQ(v, "abcabc");
        }
        {
            auto m = ks->match("abcdabc");
            ASSERT_EQ(m != nullptr, true);
            std::string v = m->str();
            EXPECT_EQ(v, "abc");
        }
    }
    // EXPECT_EQ(KParser::KObject::count, 0);
}

TEST(TEST_PARSER, MANY2) {
    // EXPECT_EQ(KParser::KObject::count, 0);
    {
        KParser::Parser p;

        auto k = p.any(
            p.str("abc"),
            p.str("123")
        );
        auto ks = p.many(k);
        {
            auto m = ks->match("abc");
            std::string v = m->str();
            EXPECT_EQ(v, "abc");
        }
        {
            auto m = ks->match("abc123");
            std::string v = m->str();
            EXPECT_EQ(v, "abc123");
        }
        {
            auto m = ks->match("abc 123abc abc 12312");
            std::string v = m->str();
            EXPECT_EQ(v, "abc 123abc abc 123");
        }
    }
}

TEST(TEST_PARSER, MANY3) {
    // EXPECT_EQ(KParser::KObject::count, 0);
    {
        KParser::Parser p;
        
        auto k = p.any(
            p.str("abc"),
            p.str("123")
        );
        auto ks = p.many1(k);
        {
            auto m = ks->match("");
            EXPECT_EQ(m, nullptr);
        }
        {
            auto m = ks->match("abc");
            EXPECT_EQ(m != nullptr, true);
            std::string v = m->str();
            EXPECT_EQ(v, "abc");
        }
        {
            auto m = ks->match("abc123");
            EXPECT_EQ(m != nullptr, true);
            std::string v = m->str();
            EXPECT_EQ(v, "abc123");
        }
        {
            auto m = ks->match("  abc 123abc abc 12312");
            EXPECT_EQ(m != nullptr, true);
            std::string v = m->str();
            EXPECT_EQ(v, "abc 123abc abc 123");
        }
    }
    // EXPECT_EQ(KParser::KObject::count, 0);    
}

TEST(TEST_PARSER, PRED) {
    // EXPECT_EQ(KParser::KObject::count, 0);
    {
        KParser::Parser p;

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
            auto m = k->match("abdddd");
            ASSERT_EQ(m != nullptr, true);
            EXPECT_EQ(m->str(), "abddd");
            EXPECT_EQ(*m->get<std::string>(), "abddd");
        };
    }
}


TEST(TEST_PARSER, op) {
    // EXPECT_EQ(KParser::KObject::count, 0);
    {
        KParser::Parser p;
        auto r = p.optional(
            p.str("abc")
        );
        {
            auto m = r->match("");
            ASSERT_EQ(m != nullptr, true);
            EXPECT_EQ(m->str(), "");
        };
        {
            auto m = r->match("abc");
            ASSERT_EQ(m != nullptr, true);
            EXPECT_EQ(m->str(), "abc");
        };
    }
}

TEST(TEST_PARSER, until) {
    // EXPECT_EQ(KParser::KObject::count, 0);
    {
        KParser::Parser p;
        auto k = p.until(
            p.str("abc")
        );
        {
            auto m = k->match("12341234abc");
            ASSERT_EQ(m != nullptr, true);
            EXPECT_EQ(m->str(), "12341234");
        }

        {
            auto m = k->match("12341234");
            ASSERT_EQ(m == nullptr, true);
        };
    }
}

TEST(TEST_PARSER, function) {
    // EXPECT_EQ(KParser::KObject::count, 0);
    {
        KParser::Parser p;

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
            auto m = k->match("12341234abc");
            ASSERT_EQ(m != nullptr, true);
            EXPECT_EQ(m->str(), "12341234");
        }

        {
            auto m = k->match("12341234");
            ASSERT_EQ(m == nullptr, true);
        };
    }
}

TEST(REGEX, RE1) {
    KParser::Parser p;
    {
        auto r = p.regex("[0-9]+");
        auto m = r->match(" a12345bc");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "a12345");
        EXPECT_EQ(*m->get<std::string>(), "12345");
    }

    {
        auto r = p.regex("^[0-9]+");
        auto m = r->match("12345bc");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "12345");
    }
}
TEST(REGEX, RE2) {
    KParser::Parser p;
    {
        auto r = p.all(p.none(), p.regex("^[a-zA-Z_][a-zA-Z0-9_]*$"));
        auto m = r->match("0asdfasfd");
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
        auto m = r->match("  _1234_");
        ASSERT_EQ(m != nullptr, true);
        ASSERT_EQ(v, "_1234_");
        ASSERT_EQ(m->str(), "_1234_");
    }
}

TEST(COMPLICATE, T1) {
    // EXPECT_EQ(KParser::KObject::count, 0);
    {
        // EXPECT_EQ(KParser::KObject::count, 0);
        KParser::Parser p;
        
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
        func->match(t);
        int i = 0;
    }
    // EXPECT_EQ(KParser::KObject::count, 0);
}

TEST(COMPLICATE, T2) {
    // EXPECT_EQ(KParser::KObject::count, 0);
    {
        auto text = "(a, b, ( ), (c, (d)))";
        {
            KParser::Parser p;

            auto id = p.identifier();
            auto group = p.all();
            auto term = p.any(id, group);

            std::vector<std::string> ids;
            id->on([&](auto* m) {
                ids.push_back(m->str());
                });
            int count = 0;
            group->add(
                p.str("("),
                p.list(term, p.str(",")),
                p.str(")"));
            
            group->on([&](auto* m) {
                    count++;
                    });

            auto m = group->match(text);
            ASSERT_EQ(m != nullptr, true);
            EXPECT_EQ(count, 4);

            ASSERT_EQ(ids.size(), 4);
            EXPECT_EQ(ids[0], "a");
            EXPECT_EQ(ids[1], "b");
            EXPECT_EQ(ids[2], "c");
            EXPECT_EQ(ids[3], "d");
        }
    }
    // EXPECT_EQ(KParser::KObject::count, 0);
}

TEST(COMPLICATE, T3) {
    // EXPECT_EQ(KParser::KObject::count, 0);
    {
        auto text = "abc";
        std::stringstream ss;
        ss << text;
        int k = 1500;
        for (auto i = 0; i < k; ++i) {
            ss << ", " << text;
        }

        {
            KParser::Parser p;
            int i = 0;
            auto f = [&](auto* m) {
                i++;
            };
            auto m = p.list(p.str("abc")->on(f), p.str(","));
            try
            {
                m->match(ss.str());
            }
            catch (std::exception& ex)
            {
                printf("Executing SEH __except block\r\n");
            }
                
            EXPECT_EQ(i, k+1);
        }
    }
    EXPECT_EQ(KParser::KObject::count, 0);
    auto left = KParser::KObject::all;
    int i = 0;
}
