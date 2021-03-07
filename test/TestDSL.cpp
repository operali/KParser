// author: operali

#include "gtest/gtest.h"
#include "../src/kparser.h"
#include "../src/impl/ebnf.h"
#include "./conf.h"

#ifdef enable_test_dsl

TEST(DSL_BASIC, text) {
    KLib42::DSLContext ctx;
    {
        auto m = ctx.r_text->parse("");
        ASSERT_EQ(m.get() == nullptr, true);
    }

    {
        auto m = ctx.r_text->parse("'");
        ASSERT_EQ(m.get() == nullptr, true);
    }

    {
        auto m = ctx.r_text->parse("\"");
        ASSERT_EQ(m.get() == nullptr, true);
    }

    {
        auto m = ctx.r_text->parse(" ' ");
        ASSERT_EQ(m.get() == nullptr, true);
    }

    {
        auto m = ctx.r_text->parse("\"\"");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "\"\"");
    }

    {
        auto m = ctx.r_text->parse("''");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "''");
    }

    {
        auto m = ctx.r_text->parse("\"asdf\"");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "\"asdf\"");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KLib42::DSLText*)(*id))->name, "asdf");
    }

    {
        auto m = ctx.r_text->parse(R"('\1411a')");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), R"('\1411a')");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KLib42::DSLText*)(*id))->name, "a1a");
    }

    {
        auto m = ctx.r_text->parse(R"("\977aa")");
        ASSERT_EQ(m.get() == nullptr, false);
        EXPECT_EQ(m->str(), R"("\977aa")");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KLib42::DSLText*)(*id))->name, "977aa");
    }
    {
        auto m = ctx.r_text->parse(R"('\9aa')");
        ASSERT_EQ(m.get() == nullptr, false);
        EXPECT_EQ(m->str(), R"('\9aa')");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KLib42::DSLText*)(*id))->name, "9aa");
    }
    {
        auto m = ctx.r_text->parse(R"('\x611a')");
        ASSERT_EQ(m.get() == nullptr, false);
        EXPECT_EQ(m->str(), R"('\x611a')");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KLib42::DSLText*)(*id))->name, "a1a");
    }
    {
        auto m = ctx.r_text->parse(R"('\x97\x97kk')");
        ASSERT_EQ(m.get() == nullptr, false);
        EXPECT_EQ(m->str(), R"('\x97\x97kk')");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KLib42::DSLText*)(*id))->name, "\x97\x97kk");
    }
    {
        auto m = ctx.r_text->parse(R"("\\f"&&?? )");
        ASSERT_EQ(m.get() == nullptr, false);
        EXPECT_EQ(m->str(), R"("\\f")");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KLib42::DSLText*)(*id))->name, "\\f");
    }

    {
        auto m = ctx.r_text->parse("'ab\\nc'");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "'ab\\nc'");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KLib42::DSLText*)(*id))->name, "ab\nc");
    }
    
    {
        auto m = ctx.r_text->parse("'ab\dnc'");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "'ab\dnc'");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KLib42::DSLText*)(*id))->name, "abdnc");
    }

}

TEST(DSL_BASIC, regex) {
    KLib42::Parser p;
    KLib42::DSLContext ctx;
    {
        auto m = ctx.r_regex->parse("");
        ASSERT_EQ(m.get() == nullptr, true);
    }

    {
        auto m = ctx.r_regex->parse("/");
        ASSERT_EQ(m.get() == nullptr, true);
    }
    {
        auto m = ctx.r_regex->parse("//");
        ASSERT_EQ(m.get() == nullptr, true);
    }
    {
        auto m = ctx.r_regex->parse("/abc/");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "/abc/");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KLib42::DSLRegex*)(*id))->name, "abc");
    }
    {
        auto m = ctx.r_regex->parse(R"(/\//abc/)");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), R"(/\//)");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KLib42::DSLRegex*)(*id))->name, "\\/");
    }

    {
        auto m = ctx.r_regex->parse("/abcd/`");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "/abcd/");
        EXPECT_EQ(m->prefix(), "");
        EXPECT_EQ(m->suffix(), "`");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KLib42::DSLRegex*)(*id))->name, "abcd");
    }
}

TEST(DSL_BASIC, item) {
    KLib42::Parser p;
    KLib42::DSLContext ctx;
    {
        auto m = ctx.r_item->parse("abc");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "abc");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        EXPECT_EQ(&typeid(**id), &typeid(KLib42::DSLID));
        ASSERT_EQ(((KLib42::DSLID*)(*id))->name, "abc");
    }

    {
        auto m = ctx.r_item->parse("'abc'");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "'abc'");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        EXPECT_EQ(&typeid(**id), &typeid(KLib42::DSLText));
        ASSERT_EQ(((KLib42::DSLText*)(*id))->name, "abc");
    }

    {
        auto m = ctx.r_item->parse("/abc/");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "/abc/");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        EXPECT_EQ(&typeid(**id), &typeid(KLib42::DSLRegex));
        ASSERT_EQ(((KLib42::DSLRegex*)(*id))->name, "abc");
    }
}

TEST(DSL_BASIC, group) {
    KLib42::Parser p;
    KLib42::DSLContext ctx;
    {
        auto m = ctx.r_item->parse("(abc)abc");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "(abc)");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        EXPECT_EQ(&typeid(**id), &typeid(KLib42::DSLID));
        ASSERT_EQ(((KLib42::DSLID*)(*id))->name, "abc");
    }

    {
        auto m = ctx.r_item->parse("('ab\\nc')`");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "('ab\\nc')");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        EXPECT_EQ(&typeid(**id), &typeid(KLib42::DSLText));
        ASSERT_EQ(((KLib42::DSLText*)(*id))->name, "ab\nc");
    }

    {
        auto m = ctx.r_item->parse("(/abc/)");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "(/abc/)");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        EXPECT_EQ(&typeid(**id), &typeid(KLib42::DSLRegex));
        ASSERT_EQ(((KLib42::DSLRegex*)(*id))->name, "abc");
    }
}

TEST(DSL_BASIC, till) {
    KLib42::Parser p;
    KLib42::DSLContext ctx;
    {
        auto m = ctx.r_till->parse("...'abc'");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "...'abc'");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        EXPECT_EQ(&typeid(**id), &typeid(KLib42::DSLTill));
        KLib42::DSLText* node = (KLib42::DSLText*)((KLib42::DSLTill*)(*id))->node;
        ASSERT_EQ(node->name, "abc");
    }

    {
        auto m = ctx.r_till->parse("...(abc)");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "...(abc)");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        EXPECT_EQ(&typeid(**id), &typeid(KLib42::DSLTill));
        KLib42::DSLNode* node = ((KLib42::DSLTill*)(*id))->node;
        EXPECT_EQ(&typeid(*node), &typeid(KLib42::DSLID));
        KLib42::DSLID* node1 = (KLib42::DSLID*)((KLib42::DSLTill*)(node))->node;
    }
}

TEST(DSL_BASIC, all) {
    KLib42::Parser p;
    KLib42::DSLContext ctx;
    {
        auto m = ctx.r_all->parse("'abc'");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "'abc'");
        KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        EXPECT_EQ(&typeid(**id), &typeid(KLib42::DSLText));
        ASSERT_EQ(((KLib42::DSLText*)(*id))->name, "abc");
    }

    {
        auto m = ctx.r_all->parse("'abc'/abc/");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "'abc'/abc/");
        {
            KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
            ASSERT_EQ(id != nullptr, true);
            EXPECT_EQ(&typeid(**id), &typeid(KLib42::DSLAll));
            auto& children = ((KLib42::DSLAll*)(*id))->nodes;
            {
                ASSERT_EQ(((KLib42::DSLText*)(children[0]))->name, "abc");
            }
            {
                ASSERT_EQ(((KLib42::DSLRegex*)(children[1]))->name, "abc");
            }
        }
    }

    {
        auto m = ctx.r_all->parse(R"('\/\/'/abc/abc)");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), R"('\/\/'/abc/abc)");
        {
            KLib42::DSLNode** id = m->capture<KLib42::DSLNode*>(0);
            ASSERT_EQ(id != nullptr, true);
            EXPECT_EQ(&typeid(**id), &typeid(KLib42::DSLAll));
            auto& children = ((KLib42::DSLAll*)(*id))->nodes;
            {
                ASSERT_EQ(((KLib42::DSLText*)(children[0]))->name, R"(//)");
            }
            {
                ASSERT_EQ(((KLib42::DSLRegex*)(children[1]))->name, "abc");
            }
            {
                ASSERT_EQ(((KLib42::DSLID*)(children[2]))->name, "abc");
            }
        }
    }
}

TEST(DSL_BASIC, any) {
    KLib42::DSLContext ctx;
    {
        auto m = ctx.r_any->parse("'abc'");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "'abc'");
        KLib42::DSLNode** n = m->capture<KLib42::DSLNode*>(0);
        EXPECT_EQ(&typeid(**n), &typeid(KLib42::DSLText));
        
        {
            ASSERT_EQ(((KLib42::DSLText*)(*n))->name, R"(abc)");
        }
    }

    {
        auto m = ctx.r_any->parse("abc|/abc/");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), "abc|/abc/");
        KLib42::DSLNode** n = m->capture<KLib42::DSLNode*>(0);
        EXPECT_EQ(&typeid(**n), &typeid(KLib42::DSLAny));
        auto& children = ((KLib42::DSLAny*)(*n))->nodes;
        {
            ASSERT_EQ(((KLib42::DSLID*)(children[0]))->name, R"(abc)");
        }
        {
            ASSERT_EQ(((KLib42::DSLRegex*)(children[1]))->name, R"(abc)");
        }
    }

    {
            auto m = ctx.r_any->parse("abc |/abc/ abc");
            ASSERT_EQ(m.get() != nullptr, true);
            EXPECT_EQ(m->str(), "abc |/abc/ abc");
            KLib42::DSLNode** n = m->capture<KLib42::DSLNode*>(0);
            EXPECT_EQ(&typeid(**n), &typeid(KLib42::DSLAny));
            auto& children = ((KLib42::DSLAny*)(*n))->nodes;
            {
                ASSERT_EQ(((KLib42::DSLID*)(children[0]))->name, R"(abc)");
            }
            
            {
                auto* all = (KLib42::DSLAll*)(children[1]);
                auto& children1 = all->nodes;
                {
                    ASSERT_EQ(((KLib42::DSLRegex*)(children1[0]))->name, R"(abc)");
                }
                {
                    ASSERT_EQ(((KLib42::DSLID*)(children1[1]))->name, R"(abc)");
                }
            }
        }
}

TEST(DSL_BASIC, rule) {
    KLib42::DSLContext ctx;
    {
        auto m = ctx.r_rule->parse(R"(a = a '\/\/' /[1-9]*/ | b;  )");
        auto err = ctx.m_parser.getErrInfo();
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), R"(a = a '\/\/' /[1-9]*/ | b;)");
        KLib42::DSLNode** pN = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(pN != nullptr, true);
        auto* n = (KLib42::DSLRule*)*pN;
        EXPECT_EQ(n->name, "a");}
}

TEST(DSL_BASIC, ruleList) {
    KLib42::DSLContext ctx;
    {
        auto m = ctx.r_ruleList->parse(R"(
// comment 1
// comment 2
a = a a | b;
/*second comments * / */b = a | b;
c = c | /d*/;
)");
        ASSERT_EQ(m.get() != nullptr, true);
        EXPECT_EQ(m->str(), R"(// comment 1
// comment 2
a = a a | b;
/*second comments * / */b = a | b;
c = c | /d*/;)");

        KLib42::DSLNode** d = m->capture<KLib42::DSLNode*>(0);
        ASSERT_EQ(d != nullptr, true);
        EXPECT_EQ(&typeid(**d), &typeid(KLib42::DSLRuleList));
        KLib42::DSLRuleList* rlist = (KLib42::DSLRuleList*)*d;
        EXPECT_EQ(rlist->nodes.size(), 3);
    }

    {
        auto m = ctx.r_ruleList->parse(R"(a d = 3
// comment 1
// comment 2
a = a a | b;
/*second comments * / */b = a | b;
c = c | /d*/;
)");
        ASSERT_EQ(m.get() == nullptr, true);
        auto err = ctx.m_parser.getErrInfo();
        ASSERT_EQ(!!err, true);
        std::cerr << err->message() << std::endl;
    }
}

TEST(DSL_BASIC, ruleList_parse) {
    
    {
        KLib42::DSLContext ctx;
        ctx.prepareRules(R"(
a = ID | NUM;
b = a+ EOF;
)");
        int numVal = 0;
        std::string strVal= "";
        int count = 0;
        ctx.prepareCapture("a", [&](KLib42::Match& m, KLib42::IT arg, KLib42::IT noarg)->KLib42::KAny {
            double* pNum = arg->get<double>();
            if (pNum) {
                double num = *pNum;
                std::cout << "number: " << num << std::endl;
                numVal += num;
                return num;
            }
            else {
                auto* pid = arg->get<std::string>();
                if (pid) {
                    auto id = *pid;
                    strVal += id;
                    std::cout << "string: " << id << std::endl;
                    return id;
                }
                return nullptr;
            }
            
        });
        ctx.prepareCapture("b", [&](KLib42::Match& m, KLib42::IT arg, KLib42::IT noarg) {
            for (; arg != noarg; ++arg) {
                count++;
            }
            return nullptr;
            });
        auto r = ctx.build();
        if (!r) {
            std::cerr << ctx.lastError->message() << std::endl;
        }
        else {
            ctx.parse("b", "11 22 aa bb");
            EXPECT_EQ(numVal, 33);
            EXPECT_EQ(strVal, "aabb");
            EXPECT_EQ(count, 4);
        }
    }
    {
        KLib42::DSLContext ctx;
        ctx.prepareRules(R"(  a = a | b;
b = a | b;
dd xxtt
c = re | /\/\//;
)");
        auto r = ctx.build();
        ASSERT_EQ(r, false);
        if (!r) {
            std::cerr << ctx.lastError->message() << std::endl;
        }
    }
    {
        EXPECT_EQ(KLib42::KObject::count, 0);
        KLib42::DSLContext ctx;
        ctx.prepareRules(R"(  a = ID;
a = NUM;
)");
        auto r = ctx.build();
        ASSERT_EQ(r, false);
        if (!r) {
            std::cerr << ctx.lastError->message() << std::endl;
        }
    }
    
    // auto& objs = KLib42::KObject::debug();
    EXPECT_EQ(KLib42::KObject::count, 0);
}

TEST(DSL_BASIC, list) {
    KLib42::EasyParser p;
    
    {
        double numval = 0;
        std::string strval = "";
        p.prepareRules(R"(
a = ID | NUM;
b = [a ','] EOF;
)");
        p.prepareCapture("a", [&](KLib42::Match& m, KLib42::IT arg, KLib42::IT noarg) {
            double* pNum = arg->get<double>();
            if (pNum) {
                double num = *pNum;
                std::cout << "number of" << num << std::endl;
                numval += num;
                return nullptr;
            }
            else {
                std::string* pid = arg->get<std::string>();
                if (pid) {
                    auto id = *pid;
                    strval += id;
                    std::cout << "string of" << id << std::endl;
                    return nullptr;
                }
                else {
                    return nullptr;
                }

            }
            });
        auto r = p.build();
        if (!r) {
            std::cerr << p.getLastError()->message() << std::endl;
            ASSERT_EQ(false, true);
        }
        else {
            p.parse("b", "11, 22 ,aa, bb");
            EXPECT_EQ(numval, 33);
            EXPECT_EQ(strval, "aabb");
        }
    }
}


TEST(DSL, till) {
    KLib42::EasyParser p;

    {
        double numval = 0;
        std::string strval = "";
        p.prepareRules(R"(
a = ID | NUM;
b = (...a)* EOF;
)");
        p.prepareCapture("a", [&](KLib42::Match& m, KLib42::IT arg, KLib42::IT noarg) {
            double* pNum = arg->get<double>();
            if (pNum) {
                double num = *pNum;
                std::cout << "number of" << num << std::endl;
                numval += num;
                return nullptr;
            }
            else {
                std::string* pid = arg->get<std::string>();
                if (pid) {
                    auto id = *pid;
                    strval += id;
                    std::cout << "string of" << id << std::endl;
                    return nullptr;
                }
                else {
                    return nullptr;
                }

            }
            });
        auto r = p.build();
        if (!r) {
            std::cerr << p.getLastError()->message() << std::endl;
            ASSERT_EQ(false, true);
        }
        else {
            p.parse("b", "11, 22 ,aa, bb");
            EXPECT_EQ(numval, 33);
            EXPECT_EQ(strval, "aabb");
        }
    }
}

TEST(DSL, event) {
    KLib42::EasyParser p;

    {
        double numval = 0;
        std::string strval = "";
        p.prepareRules(R"(
a = ID | NUM;
b = (...(a@abc))* EOF;
)");
        p.prepareCapture("abc", [&](KLib42::Match& m, KLib42::IT arg, KLib42::IT noarg) {
            double* pNum = arg->get<double>();
            if (pNum) {
                double num = *pNum;
                std::cout << "number of" << num << std::endl;
                numval += num;
                return nullptr;
            }
            else {
                std::string* pid = arg->get<std::string>();
                if (pid) {
                    auto id = *pid;
                    strval += id;
                    std::cout << "string of" << id << std::endl;
                    return nullptr;
                }
                else {
                    return nullptr;
                }

            }
            });
        auto r = p.build();
        if (!r) {
            std::cerr << p.getLastError()->message() << std::endl;
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