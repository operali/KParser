#include "gtest/gtest.h"
#include "../src/KParser.h"
#include "../src/impl/dsl.h"

#define X

#ifdef X
TEST(DSL_BASIC, text) {
    KParser::DSLContext ctx;
    {
        auto m = ctx.r_text->parse("");
        ASSERT_EQ(m == nullptr, true);
    }
    {
        auto m = ctx.r_text->parse("'");
        ASSERT_EQ(m == nullptr, true);
    }
    {
        auto m = ctx.r_text->parse("\"");
        ASSERT_EQ(m == nullptr, true);
    }
    {
        auto m = ctx.r_text->parse(" ' ");
        ASSERT_EQ(m == nullptr, true);
    }
    {
        auto m = ctx.r_text->parse("\"\"");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "\"\"");
    }
    {
        auto m = ctx.r_text->parse("''");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "''");
    }
    {
        auto m = ctx.r_text->parse("\"asdf\"");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "\"asdf\"");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "asdf");
    }

    {
        auto m = ctx.r_text->parse(R"('\1411a')");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), R"('\1411a')");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "a1a");
    }
    {
        auto m = ctx.r_text->parse(R"("\977aa")");
        ASSERT_EQ(m == nullptr, false);
        EXPECT_EQ(m->str(), R"("\977aa")");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "977aa");
    }
    {
        auto m = ctx.r_text->parse(R"('\9aa')");
        ASSERT_EQ(m == nullptr, false);
        EXPECT_EQ(m->str(), R"('\9aa')");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "9aa");
    }
    {
        auto m = ctx.r_text->parse(R"('\x611a')");
        ASSERT_EQ(m == nullptr, false);
        EXPECT_EQ(m->str(), R"('\x611a')");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "a1a");
    }
    {
        auto m = ctx.r_text->parse(R"('\x97\x97kk')");
        ASSERT_EQ(m == nullptr, false);
        EXPECT_EQ(m->str(), R"('\x97\x97kk')");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "\x97\x97kk");
    }
    {
        auto m = ctx.r_text->parse(R"("\\f"&&?? )");
        ASSERT_EQ(m == nullptr, false);
        EXPECT_EQ(m->str(), R"("\\f")");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "\\f");
    }

    {
        auto m = ctx.r_text->parse("'ab\\nc'");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "'ab\\nc'");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "ab\nc");
    }
    
    {
        auto m = ctx.r_text->parse("'ab\dnc'");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "'ab\dnc'");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "abdnc");
    }

}

TEST(DSL_BASIC, regex) {
    KParser::Parser p;
    KParser::DSLContext ctx;
    {
        auto m = ctx.r_regex->parse("");
        ASSERT_EQ(m == nullptr, true);
    }

    {
        auto m = ctx.r_regex->parse("/");
        ASSERT_EQ(m == nullptr, true);
    }
    {
        auto m = ctx.r_regex->parse("//");
        ASSERT_EQ(m == nullptr, true);
    }
    {
        auto m = ctx.r_regex->parse("/abc/");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "/abc/");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLRegex*)(*id))->name, "abc");
    }
    {
        auto m = ctx.r_regex->parse(R"(/\//abc/)");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), R"(/\//)");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLRegex*)(*id))->name, "\\/");
    }

    {
        auto m = ctx.r_regex->parse("/abcd/`");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "/abcd/");
        EXPECT_EQ(m->prefix(), "");
        EXPECT_EQ(m->suffix(), "`");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLRegex*)(*id))->name, "abcd");
    }
}

TEST(DSL_BASIC, item) {
    KParser::Parser p;
    KParser::DSLContext ctx;
    {
        auto m = ctx.r_item->parse("abc");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "abc");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        EXPECT_EQ(&typeid(**id), &typeid(KParser::DSLID));
        ASSERT_EQ(((KParser::DSLID*)(*id))->name, "abc");
    }

    {
        auto m = ctx.r_item->parse("'abc'");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "'abc'");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        EXPECT_EQ(&typeid(**id), &typeid(KParser::DSLText));
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "abc");
    }

    {
        auto m = ctx.r_item->parse("/abc/");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "/abc/");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        EXPECT_EQ(&typeid(**id), &typeid(KParser::DSLRegex));
        ASSERT_EQ(((KParser::DSLRegex*)(*id))->name, "abc");
    }
}

TEST(DSL_BASIC, group) {
    KParser::Parser p;
    KParser::DSLContext ctx;
    {
        auto m = ctx.r_item->parse("(abc)abc");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "(abc)");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        EXPECT_EQ(&typeid(**id), &typeid(KParser::DSLID));
        ASSERT_EQ(((KParser::DSLID*)(*id))->name, "abc");
    }

    {
        auto m = ctx.r_item->parse("('ab\\nc')`");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "('ab\\nc')");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        EXPECT_EQ(&typeid(**id), &typeid(KParser::DSLText));
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "ab\nc");
    }

    {
        auto m = ctx.r_item->parse("(/abc/)");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "(/abc/)");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        EXPECT_EQ(&typeid(**id), &typeid(KParser::DSLRegex));
        ASSERT_EQ(((KParser::DSLRegex*)(*id))->name, "abc");
    }
}

TEST(DSL_BASIC, all) {
    KParser::Parser p;
    KParser::DSLContext ctx;
    {
        auto m = ctx.r_all->parse("'abc'");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "'abc'");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        EXPECT_EQ(&typeid(**id), &typeid(KParser::DSLText));
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "abc");
    }

    {
        auto m = ctx.r_all->parse("'abc'/abc/");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "'abc'/abc/");
        {
            KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
            ASSERT_EQ(id != nullptr, true);
            EXPECT_EQ(&typeid(**id), &typeid(KParser::DSLAll));
            auto& children = ((KParser::DSLAll*)(*id))->nodes;
            {
                ASSERT_EQ(((KParser::DSLText*)(children[0]))->name, "abc");
            }
            {
                ASSERT_EQ(((KParser::DSLRegex*)(children[1]))->name, "abc");
            }
        }
    }

    {
        auto m = ctx.r_all->parse(R"('\/\/'/abc/abc)");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), R"('\/\/'/abc/abc)");
        {
            KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
            ASSERT_EQ(id != nullptr, true);
            EXPECT_EQ(&typeid(**id), &typeid(KParser::DSLAll));
            auto& children = ((KParser::DSLAll*)(*id))->nodes;
            {
                ASSERT_EQ(((KParser::DSLText*)(children[0]))->name, R"(//)");
            }
            {
                ASSERT_EQ(((KParser::DSLRegex*)(children[1]))->name, "abc");
            }
            {
                ASSERT_EQ(((KParser::DSLID*)(children[2]))->name, "abc");
            }
        }
    }
}

TEST(DSL_BASIC, any) {
    KParser::DSLContext ctx;
    {
        auto m = ctx.r_any->parse("'abc'");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "'abc'");
        KParser::DSLNode** n = m->capture_s<KParser::DSLNode*>(0);
        EXPECT_EQ(&typeid(**n), &typeid(KParser::DSLText));
        
        {
            ASSERT_EQ(((KParser::DSLText*)(*n))->name, R"(abc)");
        }
    }

    {
        auto m = ctx.r_any->parse("abc|/abc/");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "abc|/abc/");
        KParser::DSLNode** n = m->capture_s<KParser::DSLNode*>(0);
        EXPECT_EQ(&typeid(**n), &typeid(KParser::DSLAny));
        auto& children = ((KParser::DSLAny*)(*n))->nodes;
        {
            ASSERT_EQ(((KParser::DSLID*)(children[0]))->name, R"(abc)");
        }
        {
            ASSERT_EQ(((KParser::DSLRegex*)(children[1]))->name, R"(abc)");
        }
    }

    {
            auto m = ctx.r_any->parse("abc |/abc/ abc");
            ASSERT_EQ(m != nullptr, true);
            EXPECT_EQ(m->str(), "abc |/abc/ abc");
            KParser::DSLNode** n = m->capture_s<KParser::DSLNode*>(0);
            EXPECT_EQ(&typeid(**n), &typeid(KParser::DSLAny));
            auto& children = ((KParser::DSLAny*)(*n))->nodes;
            {
                ASSERT_EQ(((KParser::DSLID*)(children[0]))->name, R"(abc)");
            }
            
            {
                auto* all = (KParser::DSLAll*)(children[1]);
                auto& children1 = all->nodes;
                {
                    ASSERT_EQ(((KParser::DSLRegex*)(children1[0]))->name, R"(abc)");
                }
                {
                    ASSERT_EQ(((KParser::DSLID*)(children1[1]))->name, R"(abc)");
                }
            }
        }
}

TEST(DSL_BASIC, rule) {
    KParser::DSLContext ctx;
    {
        auto m = ctx.r_rule->parse(R"(a = a '\/\/' /[1-9]*/ | b;  )");
        auto err = ctx.m_parser.errInfo();
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), R"(a = a '\/\/' /[1-9]*/ | b;)");
        KParser::DSLNode** pN = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(pN != nullptr, true);
        auto* n = (KParser::DSLRule*)*pN;
        EXPECT_EQ(n->name, "a");}
}

TEST(DSL_BASIC, ruleList) {
    KParser::DSLContext ctx;
    {
        auto m = ctx.r_ruleList->parse(R"(  a = a a | b;
b = a | b;
c = c | /d*/;
)");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), R"(a = a a | b;
b = a | b;
c = c | /d*/;)");

        KParser::DSLNode** d = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(d != nullptr, true);
        EXPECT_EQ(&typeid(**d), &typeid(KParser::DSLRuleList));
        KParser::DSLRuleList* rlist = (KParser::DSLRuleList*)*d;
        EXPECT_EQ(rlist->nodes.size(), 3);
    }
}

TEST(DSL_BASIC, ruleList_parse) {
    KParser::DSLContext ctx;
    {
        ctx.prepareRules(R"(
a = ID | NUM;
b = a+ EOF;
)");
        int numVal = 0;
        std::string strVal= "";
        int count = 0;
        ctx.prepareEvaluation("a", [&](KParser::Match& m, KParser::IT arg, KParser::IT noarg) {
            try {
                int num = libany::any_cast<int>(*arg);
                std::cout << "number: " << num << std::endl;
                numVal += num;
                return nullptr;
            }
            catch (libany::bad_any_cast & ex) {
                try {
                    auto id = libany::any_cast<std::string>(*arg);
                    strVal += id;
                    std::cout << "string: " << id << std::endl;
                    return nullptr;
                }
                catch (libany::bad_any_cast & ex) {

                }
            }
            return nullptr;
        });
        ctx.prepareEvaluation("b", [&](KParser::Match& m, KParser::IT arg, KParser::IT noarg) {
            for (; arg != noarg; ++arg) {
                count++;
            }
            return nullptr;
            });
        auto r = ctx.build();
        if (!r) {
            std::cerr << ctx.lastError << std::endl;
        }
        else {
            ctx.parse("b", "11 22 aa bb");
            EXPECT_EQ(numVal, 33);
            EXPECT_EQ(strVal, "aabb");
            EXPECT_EQ(count, 4);
        }
    }
    {
        
        ctx.prepareRules(R"(  a = a | b;
b = a | b;
dd
c = re | /\/\//;
)");
        auto r = ctx.build();
        ASSERT_EQ(r, false);
        if (!r) {
            std::cerr << ctx.lastError << std::endl;
        }
    }
}

TEST(DSL_BASIC, list) {
    KParser::EasyParser p;
    
    {
        int numval = 0;
        std::string strval = "";
        p.prepareRules(R"(
a = ID | NUM;
b = [a ','] EOF;
)");
        p.prepareEvaluation("a", [&](KParser::Match& m, KParser::IT arg, KParser::IT noarg) {
                        try {
                            int num = libany::any_cast<int>(*arg);
                            std::cout << "number of" << num << std::endl;
                            numval += num;
                            return nullptr;
                        }
                        catch (libany::bad_any_cast & ex) {
                            try {
                                auto id = libany::any_cast<std::string>(*arg);
                                strval += id;
                                std::cout << "string of" << id << std::endl;
                                return nullptr;
                            }
                            catch (libany::bad_any_cast & ex) {
            
                            }
                        }
                        return nullptr;
                    });
        auto r = p.build();
        if (!r) {
            std::cerr << p.getLastError()<< std::endl;
            ASSERT_EQ(false, true);
        }
        else {
            p.parse("b", "11, 22 ,aa, bb");
            EXPECT_EQ(numval, 33);
            EXPECT_EQ(strval, "aabb");
        }
    }
}

#endif