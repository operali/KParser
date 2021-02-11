#include "gtest/gtest.h"
#include "../src/KParser.h"
#include "../src/impl/dsl.h"

// #define X

#ifndef X
TEST(DSL_BASIC, text) {
    KParser::DSLContext ctx;
    {
        auto m = ctx.r_text->parse("");
        ASSERT_EQ(m == nullptr, true);
    }

    {
        auto m = ctx.r_text->parse(" ");
        ASSERT_EQ(m == nullptr, true);
    }

    {
        auto m = ctx.r_text->parse("asdf");
        ASSERT_EQ(m != nullptr, true);
        EXPECT_EQ(m->str(), "asdf");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "asdf");
    }
    {
        auto m = ctx.r_text->parse(R"(\1411a )");
        ASSERT_EQ(m == nullptr, false);
        EXPECT_EQ(m->str(), R"(\1411a)");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "a1a");
    }
    {
        auto m = ctx.r_text->parse(R"(\977aa )");
        ASSERT_EQ(m == nullptr, false);
        EXPECT_EQ(m->str(), R"(\977aa)");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "977aa");
    }
    {
        auto m = ctx.r_text->parse(R"(\9aa )");
        ASSERT_EQ(m == nullptr, false);
        EXPECT_EQ(m->str(), R"(\9aa)");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "9aa");
    }
    {
        auto m = ctx.r_text->parse(R"(\x611a )");
        ASSERT_EQ(m == nullptr, false);
        EXPECT_EQ(m->str(), R"(\x611a)");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "a1a");
    }
    {
        auto m = ctx.r_text->parse(R"(\x97\x97kk )");
        ASSERT_EQ(m == nullptr, false);
        EXPECT_EQ(m->str(), R"(\x97\x97kk)");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "\x97\x97kk");
    }
    {
        auto m = ctx.r_text->parse(R"(\\f&&?? )");
        ASSERT_EQ(m == nullptr, false);
        EXPECT_EQ(m->str(), R"(\\f&&)");
        KParser::DSLNode** id = m->capture_s<KParser::DSLNode*>(0);
        ASSERT_EQ(id != nullptr, true);
        ASSERT_EQ(((KParser::DSLText*)(*id))->name, "\\f&&");
    }


    //
    //{
    //    auto m = ctx.r_text->parse("`abc`");
    //    ASSERT_EQ(m != nullptr, true);
    //    EXPECT_EQ(m->str(), "`abc`");
    //    DSLNode** id = m->capture_s<DSLNode*>(0);
    //    ASSERT_EQ(id != nullptr, true);
    //    ASSERT_EQ(((DSLID*)(*id))->name, "abc");
    //}
}

//TEST(DSL_BASIC, regex) {
//    KParser::Parser p;
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
//        EXPECT_EQ(m->str(), "re``");
//        DSLNode** id = m->capture_s<DSLNode*>(0);
//        ASSERT_EQ(id != nullptr, true);
//        ASSERT_EQ(((DSLRegex*)(*id))->name, "");
//    }
//
//    {
//        auto m = ctx.r_regex->parse("re`abcd``");
//        ASSERT_EQ(m != nullptr, true);
//        EXPECT_EQ(m->str(), "re`abcd`");
//        EXPECT_EQ(m->prefix(), "");
//        EXPECT_EQ(m->suffix(), "`");
//        DSLNode** id = m->capture_s<DSLNode*>(0);
//        ASSERT_EQ(id != nullptr, true);
//        ASSERT_EQ(((DSLRegex*)(*id))->name, "abcd");
//    }
//}
//
//TEST(DSL_BASIC, item) {
//    KParser::Parser p;
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
//    KParser::Parser p;
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
//    KParser::Parser p;
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
//
//TEST(DSL_BASIC, ruleList_parse) {
//    DSLContext ctx;
//    {
//        auto r = ctx.ruleOf(R"(
//a@on_a = ID | NUM;
//b@on_b = a+ EOF;
//)");
//        int numVal = 0;
//        std::string strVal= "";
//        int count = 0;
//        ctx.bind("on_a", [&](KParser::Match& m, KParser::IT arg, KParser::IT noarg) {
//            try {
//                int num = libany::any_cast<int>(*arg);
//                std::cout << "number of" << num << std::endl;
//                numVal += num;
//                return nullptr;
//            }
//            catch (libany::bad_any_cast & ex) {
//                try {
//                    auto id = libany::any_cast<std::string>(*arg);
//                    strVal += id;
//                    std::cout << "string of" << id << std::endl;
//                    return nullptr;
//                }
//                catch (libany::bad_any_cast & ex) {
//
//                }
//            }
//            return nullptr;
//        });
//        ctx.bind("on_b", [&](KParser::Match& m, KParser::IT arg, KParser::IT noarg) {
//            for (; arg != noarg; ++arg) {
//                count++;
//            }
//            return nullptr;
//            });
//
//        if (!r) {
//            std::cerr << ctx.lastError << std::endl;
//        }
//        else {
//            ctx.parse("b", "11 22 aa bb");
//            EXPECT_EQ(numVal, 33);
//            EXPECT_EQ(strVal, "aabb");
//            EXPECT_EQ(count, 4);
//        }
//    }
//    {
//        
//        auto r = ctx.ruleOf(R"(  a = a | b;
//b = a | b;
//dd
//c = re | re`re`;
//)");
//        ASSERT_EQ(r, false);
//        if (!r) {
//            std::cerr << ctx.lastError << std::endl;
//        }
//    }
//}
//
//TEST(DSL_BASIC, list) {
//    KParser::EasyParser p;
//    
//    {
//        int numval = 0;
//        std::string strval = "";
//        auto r = p.buildRules(R"(
//a@on_a = ID | NUM;
//b@on_b = [a `,`] EOF;
//)");
//        p.bind("on_a", [&](KParser::Match& m, KParser::IT arg, KParser::IT noarg) {
//                        try {
//                            int num = libany::any_cast<int>(*arg);
//                            std::cout << "number of" << num << std::endl;
//                            numval += num;
//                            return nullptr;
//                        }
//                        catch (libany::bad_any_cast & ex) {
//                            try {
//                                auto id = libany::any_cast<std::string>(*arg);
//                                strval += id;
//                                std::cout << "string of" << id << std::endl;
//                                return nullptr;
//                            }
//                            catch (libany::bad_any_cast & ex) {
//            
//                            }
//                        }
//                        return nullptr;
//                    });
//        if (!r) {
//            std::cerr << p.getLastError()<< std::endl;
//            ASSERT_EQ(false, true);
//        }
//        else {
//            p.parse("b", "11, 22 ,aa, bb");
//            EXPECT_EQ(numval, 33);
//            EXPECT_EQ(strval, "aabb");
//        }
//    }
//}

#endif