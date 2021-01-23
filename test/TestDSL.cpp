#include "gtest/gtest.h"
#include "../src/KParser.h"
#include "../src/impl/dsl.h"


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


#else

TEST(DSL_BASIC, str) {
     {
         DSLParser dsl;

         std::vector<std::string> v;
         auto r = dsl.str_()->on([&](auto& m) {
             v.push_back(m.str());
             });

         auto m = dsl.many1(r)->parse("`abc`");
         EXPECT_EQ(v[0], "abc");

         v.clear();
         m = dsl.many1(r)->parse(" `11 1` `` `  da`");
         EXPECT_EQ(v[0], "11 1");
         EXPECT_EQ(v[1], "");
         EXPECT_EQ(v[2], "  da");
     }
 }


TEST(DSL_BASIC, recommend_) {
     {
         DSLParser dsl;

         std::vector<std::string> v;
         auto r = dsl.recommend_()->on([&](auto& m) {
             v.push_back(m.str());
             });

         auto m = dsl.many1(r)->parse(" /**/ ");
         EXPECT_EQ(v[0], "/**/");

         v.clear();
         m = dsl.many1(r)->parse(" /* abc/* */ /* asdfasdf****/ /*");
         EXPECT_EQ(v[0], "/* abc/* */");
         EXPECT_EQ(v[1], "/* asdfasdf****/");
     }
 }


 TEST(DSL_BASIC, regex_) {
     {
         DSLParser dsl;

         std::vector<std::string> v;
         auto r = dsl.regex_()->on([&](auto& m) {
             v.push_back(m.str());
             });

         auto m = dsl.many1(r)->parse(" re`` ");
         EXPECT_EQ(v[0], "re``");

         v.clear();
         m = dsl.many1(r)->parse("re`re` re`\\//abc `");
         EXPECT_EQ(v[0], "re`re`");
         EXPECT_EQ(v[1], "re`\\//abc `");
     }
 }

 TEST(DSL, term) {
     {
         DSLParser dsl;
         std::vector<std::string> v;
         auto r = dsl._term_->on([&](auto& m) {
             v.push_back(m.str());
             });
         bool visitGroup = false;
         dsl._group_->on([&](auto& m) {
             visitGroup = true;
             });
         bool visitAny = false;
         dsl._any_->on([&](auto& m) {
             visitAny = true;
             });
         bool visitAll = false;
         dsl._all_->on([&](auto& m) {
             visitAll = true;
             });
         auto m = dsl.many1(r)->parse("(abc)");
         EXPECT_EQ(v[0], "abc");
         EXPECT_EQ(v[1], "(abc)");

     }
     {
         DSLParser dsl;

         std::vector<std::string> v;
         auto r = dsl._term_->on([&](auto& m) {
             v.push_back(m.str());
             });

         auto m = dsl.many1(r)->parse("1234 `abc` re`` 567 aaa abc");
         EXPECT_EQ(v[0], "1234");
         EXPECT_EQ(v[1], "`abc`");
         EXPECT_EQ(v[2], "re``");
         EXPECT_EQ(v[3], "567");
         EXPECT_EQ(v[4], "aaa");
         EXPECT_EQ(v[5], "abc");
     }

     {
         DSLParser dsl;

         std::vector<std::string> v;
         auto r = dsl._term_->on([&](auto& m) {
             v.push_back(m.str());
             });
         bool visitGroup = false;
         dsl._group_->on([&](auto& m) {
             visitGroup = true;
             });
         auto m = dsl.many1(r)->parse("1234 `abc` re`` 567 aaa (abc)");
         EXPECT_EQ(v[0], "1234");
         EXPECT_EQ(v[1], "`abc`");
         EXPECT_EQ(v[2], "re``");
         EXPECT_EQ(v[3], "567");
         EXPECT_EQ(v[4], "aaa");
         EXPECT_EQ(v[5], "abc");
         EXPECT_EQ(v[6], "(abc)");
     }

     {
         DSLParser dsl;

         std::vector<std::string> v;
         auto r = dsl._any_->on([&](auto& m) {
             v.push_back(m.str());
             });

         auto m = r->parse("1234 | `abc` | re`` 567 aaa (abc)  ");
         EXPECT_EQ(m->str(), "1234 | `abc` | re`` 567 aaa (abc)");
     }
 }



TEST(DSL, example_1) {
     auto strRule = R"(d=d|d|d|d;
d=d d;
d=d | dd | d a b;
d=`(`d`)`;
k = x |(y))";
     {
         DSLParser dsl;

         std::vector<std::string> v;
         auto r = dsl._root_;
         dsl._rule_->on([&](auto& m) {
             v.push_back(m.str());
             });

         auto m = r->parse(strRule);
         int i = 1;

         ASSERT_EQ(v.size(), 4);
     }
 }

#endif