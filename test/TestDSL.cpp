#include "gtest/gtest.h"
#include "../src/KParser.h"
#include "../src/impl/dsl.h"

//#define X
#ifndef X
//TEST(DSL_BASIC, text) {
//    DSLContext ctx;
//    {
//        auto m = ctx.r_text->parse("");
//        ASSERT_EQ(m != nullptr, false);
//    }
//
//    {
//        auto m = ctx.r_text->parse("`");
//        ASSERT_EQ(m != nullptr, false);
//    }
//
//    {
//        auto m = ctx.r_text->parse("``");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), "");
//        DSLNode** id = m->capture_s<DSLNode*>(0);
//        ASSERT_EQ(id != nullptr, true);
//        ASSERT_EQ(((DSLID*)(*id))->name, "");
//    }
//    
//    {
//        auto m = ctx.r_text->parse("`abc`");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), "abc");
//        DSLNode** id = m->capture_s<DSLNode*>(0);
//        ASSERT_EQ(id != nullptr, true);
//        ASSERT_EQ(((DSLID*)(*id))->name, "abc");
//    }
//}
//
//TEST(DSL_BASIC, regex) {
//    DSLContext ctx;
//    {
//        auto m = ctx.r_regex->parse("");
//        ASSERT_EQ(m != nullptr, false);
//    }
//
//    {
//        auto m = ctx.r_regex->parse("re`");
//        ASSERT_EQ(m != nullptr, false);
//    }
//
//    {
//        auto m = ctx.r_regex->parse("re``");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), "");
//        DSLNode** id = m->capture_s<DSLNode*>(0);
//        ASSERT_EQ(id != nullptr, true);
//        ASSERT_EQ(((DSLRegex*)(*id))->name, "");
//    }
//
//    {
//        auto m = ctx.r_regex->parse("re`abcd``");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), "abcd");
//        EXPECT_EQ(m->prefix(), "");
//        EXPECT_EQ(m->suffix(), "`");
//        DSLNode** id = m->capture_s<DSLNode*>(0);
//        ASSERT_EQ(id != nullptr, true);
//        ASSERT_EQ(((DSLRegex*)(*id))->name, "abcd");
//    }
//}
//
//TEST(DSL_BASIC, item) {
//    DSLContext ctx;
//    {
//        auto m = ctx.r_item->parse("abc");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), "abc");
//        DSLNode** id = m->capture_s<DSLNode*>(0);
//        ASSERT_EQ(id != nullptr, true);
//        EXPECT_EQ(&typeid(**id), &typeid(DSLID));
//        ASSERT_EQ(((DSLID*)(*id))->name, "abc");
//    }
//
//    {
//        auto m = ctx.r_item->parse("`abc`");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), "`abc`");
//        DSLNode** id = m->capture_s<DSLNode*>(0);
//        ASSERT_EQ(id != nullptr, true);
//        EXPECT_EQ(&typeid(**id), &typeid(DSLText));
//        ASSERT_EQ(((DSLText*)(*id))->name, "abc");
//    }
//
//    {
//        auto m = ctx.r_item->parse("re`abc`");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), "re`abc`");
//        DSLNode** id = m->capture_s<DSLNode*>(0);
//        ASSERT_EQ(id != nullptr, true);
//        EXPECT_EQ(&typeid(**id), &typeid(DSLRegex));
//        ASSERT_EQ(((DSLRegex*)(*id))->name, "abc");
//    }
//}
//
//TEST(DSL_BASIC, group) {
//    DSLContext ctx;
//    {
//        auto m = ctx.r_item->parse("(abc)abc");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), "(abc)");
//        DSLNode** id = m->capture_s<DSLNode*>(0);
//        ASSERT_EQ(id != nullptr, true);
//        EXPECT_EQ(&typeid(**id), &typeid(DSLID));
//        ASSERT_EQ(((DSLID*)(*id))->name, "abc");
//    }
//
//    {
//        auto m = ctx.r_item->parse("(`abc`)`");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), "(`abc`)");
//        DSLNode** id = m->capture_s<DSLNode*>(0);
//        ASSERT_EQ(id != nullptr, true);
//        EXPECT_EQ(&typeid(**id), &typeid(DSLText));
//        ASSERT_EQ(((DSLText*)(*id))->name, "abc");
//    }
//
//    {
//        auto m = ctx.r_item->parse("(re`abc`)");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), "(re`abc`)");
//        DSLNode** id = m->capture_s<DSLNode*>(0);
//        ASSERT_EQ(id != nullptr, true);
//        EXPECT_EQ(&typeid(**id), &typeid(DSLRegex));
//        ASSERT_EQ(((DSLRegex*)(*id))->name, "abc");
//    }
//}
//
//TEST(DSL_BASIC, all) {
//    DSLContext ctx;
//    {
//        auto m = ctx.r_all->parse("abc");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), "abc");
//    }
//
//    {
//        auto m = ctx.r_all->parse("abc `abc`");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), "abc `abc`");
//    }
//
//    {
//        auto m = ctx.r_all->parse("rere`abc`abc");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), "rere`abc`abc");
//    }
//}
//
//TEST(DSL_BASIC, any) {
//    DSLContext ctx;
//    {
//        auto m = ctx.r_any->parse("abc");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), "abc");
//    }
//
//    {
//        auto m = ctx.r_any->parse("abc `abc`");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), "abc `abc`");
//    }
//
//    {
//        auto m = ctx.r_any->parse("abc | `abc`abc");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), "abc | `abc`abc");
//    }
//}
//
//TEST(DSL_BASIC, rule) {
//    DSLContext ctx;
//    {
//        auto m = ctx.r_rule->parse("a@evtA = a `re` re`[1-9]*` | b;  ");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), "a@evtA = a `re` re`[1-9]*` | b;");
//        DSLNode** pN = m->capture_s<DSLNode*>(0);
//        ASSERT_EQ(pN != nullptr, true);
//        auto* n = (DSLRule*)*pN;
//        EXPECT_EQ(n->name, "a");
//        EXPECT_EQ(n->evtName, "evtA");
//    }
//}
//
//TEST(DSL_BASIC, ruleList) {
//    DSLContext ctx;
//    {
//        auto m = ctx.r_ruleList->parse(R"(  a = a `re` re`[1-9]*` | b;
//b = a | b;
//c = re | re`re`;
//)");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), R"(a = a `re` re`[1-9]*` | b;
//b = a | b;
//c = re | re`re`;)");
//
//        DSLNode** d = m->capture_s<DSLNode*>(0);
//        ASSERT_EQ(d != nullptr, true);
//        EXPECT_EQ(&typeid(**d), &typeid(DSLRuleList));
//        DSLRuleList* rlist = (DSLRuleList*)*d;
//        EXPECT_EQ(rlist->nodes.size(), 3);
//
//    }
//}

TEST(DSL_BASIC, ruleList_parse) {
    DSLContext ctx;
    {
        auto r = ctx.ruleOf(R"(  

a = ID | NUM;
b@evt = a*;
)");
        ctx.bind("evt", [&](KParser::Match& m, KParser::IT arg, KParser::IT noarg) {
            while (arg != noarg) {
                try {
                    int num = libany::any_cast<int>(*arg++);
                    std::cout << "number of" << num << std::endl;
                    continue;
                }
                catch (libany::bad_any_cast& ex) {
                    arg--;
                    try {
                        auto id = libany::any_cast<std::string>(*arg++);
                        std::cout << "string of" << id << std::endl;
                        continue;
                    }
                    catch (libany::bad_any_cast& ex) {

                    }
                }
                
            }
            
            return nullptr;
            });

        if (!r) {
            std::cerr << ctx.errInfo() << std::endl;
        }
        else {
            ctx.parse("b", "333 444 abc, efef");
        }
        
    }
    /*{
        auto m = ctx.parse(R"(  a = a `re` re`[1-9]*` | b;
b = a | b;
c = re | re`re`;
)");
    }*/
        
}

#endif