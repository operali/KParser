// author: operali

#include "gtest/gtest.h"
#include "../src/kparser.h"
#include "../src/detail/rule.h"
#include <sstream>
#include <exception>
#include "./conf.h"

#ifdef enable_test_parse

TEST(BASIC, remove_space) {
    {
        KLib42::Parser p;
        std::vector<std::string> v;
        auto k = p.many1(p.identifier()->visit([&](KLib42::Match& m, bool capture) {
            if (!capture) {
                v.push_back(m.str());
            }
            }));
        k->parse(R"(a1
b1
 c1 d1
e1 f1)");
        ASSERT_EQ(v.size(), 6);
        EXPECT_EQ(v[0], "a1");
        EXPECT_EQ(v[1], "b1");
        EXPECT_EQ(v[2], "c1");
        EXPECT_EQ(v[3], "d1");
        EXPECT_EQ(v[4], "e1");
        EXPECT_EQ(v[5], "f1");

    }
}

TEST(BASIC, class_) {
    EXPECT_EQ(KLib42::KObject::count, 0);
    class MyParser : public KLib42::Parser {
    };
    {
        MyParser parser;
        EXPECT_EQ(KLib42::KObject::count, 1);
    }
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(BASIC, class1_) {
    struct MyParser : public KLib42::Parser {
        std::string val;
        KLib42::Rule* ruleOf() {
            return any(
                "abc",
                str("123")->visit([this](KLib42::Match& m, bool capture) {
                    std::cout << "hello" << std::endl;
                    this->val = m.str();
                    })
            );
        }
    };

    {
        MyParser parser;
        try {
            auto m = parser.ruleOf()->parse("123");
            EXPECT_EQ(m.get() != nullptr, true);
            EXPECT_EQ(parser.val, "123");

        }
        catch (const std::exception& _) {
            EXPECT_EQ(true, false);
        }
    }
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(BASIC, leaf_none) {
    EXPECT_EQ(KLib42::KObject::count, 0);
    {
        KLib42::Parser p;
        {
            auto r = (KLib42::RuleNode*)p.none();
            auto um = r->parse("");
            auto m = (KLib42::MatchR*)um.get();
            EXPECT_EQ(m->length(), 0);
            EXPECT_EQ(m->alter(), false);
        }
    }
    EXPECT_EQ(KLib42::KObject::count, 0);
}


TEST(BASIC, leaf_str) {

    EXPECT_EQ(KLib42::KObject::count, 0);
    {
        KLib42::Parser p;

        auto r = p.str("1234");
        {
            auto m = r->parse("1234");
            std::string v = m->str();
            EXPECT_EQ(v, "1234");
        }

        {
            EXPECT_EQ(r->parse("123").get(), nullptr);
        }

        {
            auto m = r->parse("12345");
            EXPECT_EQ(m.get() != nullptr, true);
        }
    }
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(BASIC, branch_all) {
    EXPECT_EQ(KLib42::KObject::count, 0);
    {
        KLib42::Parser p;
        {
            auto r = p.all(
                "1234",
                "5678"
            );
            auto m = r->parse("12345678");
            ASSERT_EQ(m.get() != nullptr, true);
            std::string v = m->str();
            EXPECT_EQ(v, "12345678");
        }
        {
            auto r = p.all(
                p.str("1234"),
                p.str("5678")
            );

            auto m = r->parse("12345678");
            ASSERT_EQ(m.get() != nullptr, true);
            std::string v = m->str();
            EXPECT_EQ(v, "12345678");
        }
    }
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(BASIC, branch_any1) {
    EXPECT_EQ(KLib42::KObject::count, 0);
    {
        KLib42::Parser p;
        {
            auto r = (KLib42::RuleNode*)p.any(
                p.str("1234"),
                p.str("5678")
            );
            auto um = r->parse("5678    ");
            auto pm = (KLib42::MatchR*)um.get();
            ASSERT_EQ(pm != nullptr, true);
            std::string v = pm->str();
        }
        {
            auto r = (KLib42::RuleNode*)p.any(
                "1234",
                "5678"
            );

            auto um = r->parse("5678    ");
            auto pm = (KLib42::MatchR*)um.get();
            ASSERT_EQ(pm != nullptr, true);
            std::string v = pm->str();
            EXPECT_EQ(v, "5678");
            EXPECT_EQ(pm->length(), 4);
        }
    }
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(BASIC, branch_all_any) {
    EXPECT_EQ(KLib42::KObject::count, 0);
    {
        KLib42::Parser p;
        auto k = (KLib42::RuleNode*)p.any(
            "1234",
            p.str("5678")
        );
        auto r = (KLib42::RuleNode*)p.all(
            k, k
        );
        auto m = r->parse("5678 1234");
        std::string v = m->str();
        EXPECT_EQ(v, "5678 1234");
        EXPECT_EQ(m->length(), 9);
    }
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(BASIC, branch_all_cut) {
    EXPECT_EQ(KLib42::KObject::count, 0);
    {
        // asdf123
        // asdf|asd, f123
        KLib42::Parser p;
        auto k = (KLib42::RuleNode*)p.any(
            "asdf",
            "asd"
        );
        auto k1 = p.str("f123");
        auto r = (KLib42::RuleNode*)p.all(
            k, k1
        );
        auto m = r->parse("asdf123");
        std::string v = m->str();
        EXPECT_EQ(v, "asdf123");
    }
    {
        // asdf123
        // asdf|asd, f123
        KLib42::Parser p;
        auto k = (KLib42::RuleNode*)p.any(
            "asdf",
            "asd"
        );
        auto k1 = p.str("f123");
        auto r = (KLib42::RuleNode*)p.all(
            k, p.cut(), k1
        );
        auto m = r->parse("asdf123");
        EXPECT_EQ(!!m, false);
    }
    EXPECT_EQ(KLib42::KObject::count, 0);
}


TEST(IMPLEMENT, match) {
    EXPECT_EQ(KLib42::KObject::count, 0);
    {
        KLib42::Parser p;
        auto r = (KLib42::RuleNode*)p.any(
            p.str("1234"),
            p.none()
        );
        auto m = r->parse("abcde");
        ASSERT_EQ(m->str(), "");
    }
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(FEATURE, many) {
    EXPECT_EQ(KLib42::KObject::count, 0);
    {
        KLib42::Parser p;

        auto k = p.str("abc");
        auto ks = p.many(k);
        {
            auto m = ks->parse("");
            EXPECT_EQ(m.get() != nullptr, true);
            std::string v = m->str();
            EXPECT_EQ(v, "");
        }
        {
            auto m = ks->parse("abc");
            ASSERT_EQ(m.get() != nullptr, true);
            std::string v = m->str();
            EXPECT_EQ(v, "abc");
        }
        {
            auto m = ks->parse("abcabc");
            ASSERT_EQ(m.get() != nullptr, true);
            std::string v = m->str();
            EXPECT_EQ(v, "abcabc");
        }
        {
            auto m = ks->parse("abcdabc");
            ASSERT_EQ(m.get() != nullptr, true);
            std::string v = m->str();
            EXPECT_EQ(v, "abc");
        }
    }
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(FEATURE, many_1) {
    EXPECT_EQ(KLib42::KObject::count, 0);
    {
        KLib42::Parser p;

        auto k = p.any(
            "abc",
            "123"
        );
        auto ks = p.many(k);
        {
            auto m = ks->parse("abc");
            std::string v = m->str();
            EXPECT_EQ(v, "abc");
        }
        {
            auto m = ks->parse("abc123");
            std::string v = m->str();
            EXPECT_EQ(v, "abc123");
        }
        {
            auto m = ks->parse("abc 123abc abc 12312");
            std::string v = m->str();
            EXPECT_EQ(v, "abc 123abc abc 123");
        }
    }
}

TEST(FEATURE, many1) {
    EXPECT_EQ(KLib42::KObject::count, 0);
    {
        KLib42::Parser p;

        auto k = p.any(
            "abc",
            "123"
        );
        auto ks = p.many1(k);
        {
            auto m = ks->parse("");
            EXPECT_EQ(m.get(), nullptr);
        }
        {
            auto m = ks->parse("abc");
            EXPECT_EQ(m.get() != nullptr, true);
            std::string v = m->str();
            EXPECT_EQ(v, "abc");
        }
        {
            auto m = ks->parse("abc123");
            EXPECT_EQ(m.get() != nullptr, true);
            std::string v = m->str();
            EXPECT_EQ(v, "abc123");
        }
        {
            auto m = ks->parse("  abc 123abc abc 12312");
            EXPECT_EQ(m.get() != nullptr, true);
            std::string v = m->str();
            EXPECT_EQ(v, "abc 123abc abc 123");
        }
    }
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(FEATURE, pred) {
    EXPECT_EQ(KLib42::KObject::count, 0);
    {
        KLib42::Parser p;
        auto r = p.custom([](const char* b, const char* e)->const char* {
            const char* c = b;
            int count = 0;
            while (c != e) {
                if (*c++ == 'd') {
                    count++;
                    if (count == 3) {
                        return c;
                    }
                }
            }
            return nullptr;
            });
        {
            auto m = r->parse("abdddd");
            ASSERT_EQ(m.get() != nullptr, true);
            EXPECT_EQ(m->str(), "abddd");
        };
    }
}


TEST(FEATURE, optional) {
    EXPECT_EQ(KLib42::KObject::count, 0);
    {
        KLib42::Parser p;
        auto r = p.optional(
            p.str("abc")
        );
        {
            auto m = r->parse("");
            ASSERT_EQ(m.get() != nullptr, true);
            EXPECT_EQ(m->str(), "");
        };
        {
            auto m = r->parse("abc");
            ASSERT_EQ(m.get() != nullptr, true);
            EXPECT_EQ(m->str(), "abc");
        };
    }
}

TEST(FEATURE, NOT) {
    {
        KLib42::Parser p;
        auto r = p.not_(p.str("abc"));
        {
            auto m = r->parse("abc");
            ASSERT_EQ(m.get() == nullptr, true);
        }

        {
            auto m = r->parse("ddd");
            ASSERT_EQ(m.get() != nullptr, true);
            EXPECT_EQ(m->str(), "");
        };
    }
}

TEST(FEATURE, CHAR) {
    {
        KLib42::Parser p;
        {
            auto r = p.one();
            auto m = r->parse("ab");
            ASSERT_EQ(m.get() != nullptr, true);
            ASSERT_EQ(m->str(), "a");
        }
        {
            auto r = p.all(p.one(), p.one());
            auto m = r->parse("ab");
            ASSERT_EQ(m.get() != nullptr, true);
            ASSERT_EQ(m->str(), "ab");
        }
        {
            auto r = p.all(p.one(), p.one(), p.one());
            auto m = r->parse("ab");
            ASSERT_EQ(m.get() == nullptr, true);
        }
        {
            auto r = p.many(p.all(p.not_(p.str("abc")), p.one()));
            auto m = r->parse("abc");
            ASSERT_EQ(m.get() != nullptr, true);
            ASSERT_EQ(m->str(), "");
        }
        {
            auto r = p.many(p.all(p.not_(p.str("abc")), p.one()));
            auto m = r->parse("asdfasdfabc");
            ASSERT_EQ(m.get() != nullptr, true);
            EXPECT_EQ(m->str(), "asdfasdf");
        };

        {
            auto r = p.many(p.all(p.not_(p.str("abc")), p.one()));
            auto m = r->parse("asdfasdfasdf");
            ASSERT_EQ(m.get() != nullptr, true);
            EXPECT_EQ(m->str(), "asdfasdfasdf");
        };
        {
            auto r = p.many(p.all(p.not_(p.str("abc")), p.one()));
            auto m = r->parse("asdfasdfabcasdf");
            ASSERT_EQ(m.get() != nullptr, true);
            EXPECT_EQ(m->str(), "asdfasdf");
        };
    }
}

TEST(FEATURE, until) {
    {
        KLib42::Parser p;
        auto r = p.until(
            "abc"
        );
        {
            auto m = r->parse("12341234abc");
            ASSERT_EQ(m.get() != nullptr, true);
            EXPECT_EQ(m->str(), "12341234");
        }

        {
            auto m = r->parse("12341234");
            ASSERT_EQ(m.get() == nullptr, true);
        };
    }
}

TEST(FEATURE, until_2) {
    KLib42::Parser p;
    auto to_dem = p.until(",");
    auto count = 0;
    auto dem = p.str(",")->visit([&](KLib42::Match& m, bool capture) {
        count++;
        });
    auto r = p.many(p.all(to_dem, dem));
    {
        auto m = r->parse("");
        EXPECT_EQ(count, 0);
    }
}

TEST(FEATURE, until_1) {
    KLib42::Parser p;
    auto to_dem = p.until(",");
    auto count = 0;
    auto dem = p.str(",")->visit([&](KLib42::Match& m, bool capture) {
        if (!capture) {
            count++;
        }
        });
    auto r = p.many(p.all(to_dem, dem));
    {
        auto m = r->parse(",2, asdfasfd3,4 ,,6,");
        EXPECT_EQ(count, 6);
    }
}

TEST(FEATURE, till) {
    {
        // KLib42::Match& all = KParser::KObject::debug();
        EXPECT_EQ(KLib42::KObject::count, 0);

        KLib42::Parser p;
        int count = 0;
        auto dem = p.str(",")->visit([&](KLib42::Match& m, bool capture) {
            if (!capture) {
                count++;
            }
            });
        auto r = p.many(p.till(dem));
        {
            auto m = r->parse(",2, asdfasfd3,4 ,,6,");
            EXPECT_EQ(count, 6);
        }
    }
    // KLib42::Match& all = KParser::KObject::debug();
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(FEATURE, till_1) {
    {
        KLib42::Parser p;
        int count = 0;
        {
            auto r1 = p.str("*/");
            r1->eval([&](KLib42::Match& m, KLib42::IT b, KLib42::IT e) {
                return m.str();
                });
            auto r = p.till(r1);
            auto m = r->parse("   */   */");
            EXPECT_EQ(m->str(), "   */");
            EXPECT_EQ(*m->capture<std::string>(0), "*/");
        }
    }
    // auto& all = KLib42::KObject::debug();
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(FEATURE, regex) {
    KLib42::Parser p;
    {
        auto r = p.regex("^[0-9]+");
        auto m = r->parse("12345bc");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "12345");
        EXPECT_EQ(m->str(), "12345");
    }

    {
        auto r = p.regex("[0-9]+");
        auto m = r->parse("12345bc");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "12345");
    }
}
TEST(FEATURE, regex_1) {
    {
        KLib42::Parser p;
        {
            auto r = p.all(p.none(), p.regex("^[a-zA-Z_][a-zA-Z0-9_]*$"));
            auto m = r->parse("asdfasfd");
            ASSERT_EQ(m.get() != nullptr, true);
            ASSERT_EQ(m->str(), "asdfasfd");
        }
        {

            auto r = p.all(
                p.none(),
                p.regex("[a-zA-Z_][a-zA-Z0-9_]*")->visit([&](KLib42::Match& m, bool capture) {
                    })
            );
            auto m = r->parse("  _1234_");
            ASSERT_EQ(m.get() != nullptr, true);
            ASSERT_EQ(m->str(), "_1234_");
        }
    }
    // KLib42::Match& all = KParser::KObject::debug();
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(FEATURE, identify) {
    {
        KLib42::Parser p;
        {
            auto r = p.identifier();
            auto m = r->parse("42");
            ASSERT_EQ(m.get() != nullptr, false);
        }
        {
            auto r = p.integer_();
            auto m = r->parse("42x");
            ASSERT_EQ(m.get() != nullptr, true);
            EXPECT_EQ(m->str(), "42");
        }
        {
            auto r = p.float_();
            auto m = r->parse("42.3x");
            ASSERT_EQ(m.get() != nullptr, true);
            EXPECT_EQ(m->str(), "42.3");
        }
        {
            auto r = p.float_();
            auto m = r->parse("12 ");
            ASSERT_EQ(m.get() != nullptr, true);
            EXPECT_EQ(m->str(), "12");
        }
    }
    // KLib42::Match& all = KParser::KObject::debug();
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(FEATURE, eof) {
    {
        KLib42::Parser p;
        {
            auto r = p.all(p.identifier(), p.eof());
            auto m = r->parse("ddd x");
            ASSERT_EQ(m.get() != nullptr, false);
        }
        {
            auto r = p.all(p.identifier(), p.eof());
            auto m = r->parse("ddd ");
            ASSERT_EQ(m.get() != nullptr, true);
            EXPECT_EQ(m->str(), "ddd");
        }
        {
            auto r = p.all(p.identifier(), p.integer_(), p.eof());
            auto m = r->parse("ddd 123 x");
            ASSERT_EQ(m.get() != nullptr, false);
        }

        {
            auto r = p.all(p.identifier(), p.integer_(), p.eof());
            auto m = r->parse("ddd 123 ");
            ASSERT_EQ(m.get() != nullptr, true);
            EXPECT_EQ(m->str(), "ddd 123");
        }
    }
}

TEST(example, c_function) {
    EXPECT_EQ(KLib42::KObject::count, 0);
    {
        EXPECT_EQ(KLib42::KObject::count, 0);
        KLib42::Parser p;

        bool is_cons = false;
        bool is_static = false;
        bool is_inline = false;
        std::string retType;
        auto mod = p.any(
            p.str("const")->visit([&](KLib42::Match& m, bool capture) {
                is_cons = true;
                }),
            p.str("static")->visit([&](KLib42::Match& m, bool capture) {
                    is_static = true;
                }),
                    p.str("inline")->visit([&](KLib42::Match& m, bool capture) {
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
        auto startArg = [&](KLib42::Match& m, bool capture) {
            tmpType.type = m.str();
        };

        auto stopArg = [&](KLib42::Match& m, bool capture) {
            tmpType.name = m.str();
            args.push_back(tmpType);
        };

        auto arg = p.all(
            type()->visit(startArg),
            p.identifier()->visit(stopArg)
        );

        auto func = p.all(
            p.many(mod),
            type()->visit([&](KLib42::Match& m, bool capture) {retType = m.str(); }),
            p.identifier(),
            p.str("("),
            p.list(arg, p.str(",")),
            p.str(")")
        );
        std::string t = "  const float myfun(int a, string b); ";
        func->parse(t);
        int i = 0;
    }
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(example, s_exp) {
    EXPECT_EQ(KLib42::KObject::count, 0);
    {
        auto text = "(a, b, ( ), (c, (d)))";
        {
            KLib42::Parser p;

            auto id = p.identifier();
            auto group = p.all();
            auto term = p.any(id, group);

            std::vector<std::string> ids;
            id->visit([&](KLib42::Match& m, bool capture) {
                if (!capture) {
                    ids.push_back(m.str());
                }
                
                });
            int count = 0;
            group->add(
                p.str("("),
                p.list(term, p.str(",")),
                p.str(")"));

            group->visit([&](KLib42::Match& m, bool capture) {
                if (!capture) {
                    count++;
                }
                });

            auto m = group->parse(text);
            ASSERT_EQ(m.get() != nullptr, true);
            EXPECT_EQ(count, 4);

            ASSERT_EQ(ids.size(), 4);
            EXPECT_EQ(ids[0], "a");
            EXPECT_EQ(ids[1], "b");
            EXPECT_EQ(ids[2], "c");
            EXPECT_EQ(ids[3], "d");
        }
    }
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(PRESSURE, length___) {
    // 30 (debug: 1287 ms, release:290)
    // 31 (debug: 1292 ms, release:194)
    // 01 (debug: 1187 ms, release:145)
    // 17 (debug: 1439 ms, release:160)
    EXPECT_EQ(KLib42::KObject::count, 0);
    {
        auto text = "abc";
        std::stringstream ss;
        ss << text;
        int k = 100000;
        for (auto i = 0; i < k; ++i) {
            ss << ", " << text;
        }

        {
            KLib42::Parser p;
            int i = 0;
            auto f = [&](KLib42::Match& m, bool capture) {
                i++;
            };
            auto m = p.list(p.str("abc")->visit(f), p.str(","));
            
            m->parse(ss.str());

            EXPECT_EQ(i, 2*(k + 1));
        }
    }
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(DEBUG, tostring) {
    KLib42::Parser p;
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
        auto ks = p.many("abc");
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

TEST(DEBUG, trace_back) {
    EXPECT_EQ(KLib42::KObject::count, 0);
    {
        KLib42::Parser p;
        int count = 0;
        auto counter = [&](KLib42::Match& m, bool capture) {
            count++;
        };
        auto r1 = p.many(p.str("Abc")->visit(counter));
        auto r2 = p.str("AbcAbcAbc123");
        auto r = p.all(r1, r2);
        r->parse("AbcAbcAbcAbcAbcAbcAbcAbcAbcAbc123");

        EXPECT_EQ(count, 14); // 7*2

    }
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(DEBUG, trace_back2) {
    {
        KLib42::Parser p(30);
        int count = 0;
        auto counter = [&](KLib42::Match& m, bool capture) {
            count++;
        };
        int depth = 9;
        std::stringstream ssToMatch;
        std::stringstream ssSubstr;
        for (auto i = 0; i < depth; ++i) {
            ssToMatch << "Abc";
            ssSubstr << "Abc";
        }
        ssToMatch << "AbcAb1234";
        ssSubstr << "Ab1234";

        auto r1 = p.many(p.str("Abc")->visit(counter));
        auto r2 = p.str(ssSubstr.str());
        auto r = p.all(r1, r2);
        EXPECT_EQ(r->parse(ssToMatch.str()).get() != nullptr, true);

        EXPECT_EQ(count, 2); //1*2
    }
    {
        KLib42::Parser p(30);
        int count = 0;
        auto counter = [&](KLib42::Match& m, bool capture) {
            count++;
        };
        int depth = 11;
        std::stringstream ssToMatch;
        std::stringstream ssSubstr;
        for (auto i = 0; i < depth; ++i) {
            ssToMatch << "Abc";
            ssSubstr << "Abc";
        }
        ssToMatch << "AbcAb1234";
        ssSubstr << "Ab1234";

        auto r1 = p.many(p.str("Abc")->visit(counter));
        auto r2 = p.str(ssSubstr.str());
        auto r = p.all(r1, r2);
        EXPECT_EQ(r->parse(ssToMatch.str()).get() == nullptr, true);
        EXPECT_EQ(count, 0);

    }
    EXPECT_EQ(KLib42::KObject::count, 0);
}


TEST(CAPTURE, cap1) {
    {
        KLib42::Parser p;
        
        auto* r = p.str("abc");
        r->eval([&](KLib42::Match&  m, KLib42::IT b, KLib42::IT e) {
            return m.str();
            });
        auto m = r->parse("abcabc");
        auto v = m->capture<std::string>(0);
        auto v1 = m->captureAny(0)->get<std::string>();
        EXPECT_EQ(*v, "abc");
        EXPECT_EQ(*v1, "abc");
    
        {

            auto* r = p.str("abc");
            r->eval([&](KLib42::Match& m, KLib42::IT b, KLib42::IT e) {
                return m.str();
                });
            auto* r1 = p.many1(r);
            auto m = r1->parse("abcabc");
            auto v = *m->capture<std::string>(0);
            EXPECT_EQ(v, "abc");

            auto v1 = *m->capture<std::string>(0);
            EXPECT_EQ(v1, "abc");

            auto v2 = *m->capture<std::string>(1);
            EXPECT_EQ(v2, "abc");
        }
    }
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(EVAL, NULLPTR) {
    KLib42::Parser p;
    auto* r1 = p.str("abc");
    auto* r2 = p.str("cde");
    auto* r = p.all(r1, r2);
    
    r1->eval([&](KLib42::Match& m, KLib42::IT b, KLib42::IT e) {
        return m.str();
        });
    r->eval([&](KLib42::Match& m, KLib42::IT b, KLib42::IT e) {
        return nullptr;
        });

    auto m = r->parse("abccde");
    auto v = m->captureSize();
    EXPECT_EQ(v, 0);
}
#endif