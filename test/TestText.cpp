#include "gtest/gtest.h"

// #define X

#ifdef X

#include "../src/impl/text/text.h"

TEST(TEXT, first) {
    KLib42::KText text;
    {
        text.setText("\n\nabcd\n12345");
        auto s = text.getSource();
        auto lines = s->lines();
        {
            ASSERT_EQ(lines->hasNext(), true);
            auto l = lines->next();
            EXPECT_EQ(l->str(), "");
            auto r = l->toRange();
            EXPECT_EQ(r->from(), 0);
            EXPECT_EQ(r->to(), 0);
        }

        {
            ASSERT_EQ(lines->hasNext(), true);
            auto l = lines->next();
            EXPECT_EQ(l->str(), "");
            auto r = l->toRange();
            EXPECT_EQ(r->from(), 1);
            EXPECT_EQ(r->to(), 1);
        }

        {
            ASSERT_EQ(lines->hasNext(), true);
            auto l = lines->next();
            EXPECT_EQ(l->str(), "abcd");
        }

        
        {
            EXPECT_EQ(lines->hasNext(), true);
            auto l = lines->next();
            EXPECT_EQ(l->str(), "12345");
        }
        
        {
            EXPECT_EQ(lines->hasNext(), false);
            try {
                auto l1 = lines->next();
                FAIL() << "Expected std::out_of_range";
            }
            catch (std::exception const& err) {
                SUCCEED();
            }
            catch (...) {
                FAIL() << "Expected std::out_of_range";
            }
        }
        {
            auto lines = s->lines()->toArray();
            EXPECT_EQ(lines.size(), 4);
            EXPECT_EQ(lines[0]->str(), "");
            EXPECT_EQ(lines[1]->str(), "");
            EXPECT_EQ(lines[2]->str(), "abcd");
            EXPECT_EQ(lines[3]->str(), "12345");
        }
    }

  
    
}

TEST(TEXT, second) {
    KLib42::KText text;
    {
        text.setText("abcd\n12345\n!@#$%");
        auto s = text.getSource();

    }
}

#endif