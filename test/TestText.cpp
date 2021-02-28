// author: operali

#include "gtest/gtest.h"
#include "./conf.h"


#ifdef enable_test_text

#include "../src/impl/text.h"


TEST(TEXT, second) {
    using namespace KLib42;
    {
        KShared<ISource> s(new KSource());
        {
            EXPECT_EQ(s->lineCount(), 1);
            EXPECT_EQ(s->len(), 0);
        }
        {
            s->setText("abcd\n1234\n!@#$%");
            EXPECT_EQ(s->lineCount(), 3);
            EXPECT_EQ(s->len(), 15);
            auto c0 = s->getLocation(0);
            ASSERT_EQ(c0->getRawSource(), s.operator->());
            EXPECT_EQ(c0->index(), 0);

            auto cna = s->getLocation(100);
            EXPECT_EQ(!!cna, false);

            auto line = s->getLine(0);
            EXPECT_EQ(line->str(), "abcd");
            line = line->next();
            EXPECT_EQ(line->str(), "1234");
            line = line->next();
            EXPECT_EQ(line->str(), "!@#$%");
            line = line->next();
            EXPECT_EQ(!!line, false);

            auto range = s->getRange(10, 15);
            EXPECT_EQ(range->str(), "!@#$%");
            ASSERT_EQ(range->getRawSource(), s.operator->());
            auto r = range->range();
            EXPECT_EQ(r.first, 10);
            EXPECT_EQ(r.second, 15);
            auto loc1 = s->getLocation(r.first);
            auto loc2 = s->getLocation(r.second);
            EXPECT_EQ(loc1->index(), 10);
            EXPECT_EQ(loc2->index(), 15);
            auto l1 = loc1->getLine();
            auto l2 = loc2->getLine();
            EXPECT_EQ(l1->index(), l2->index());
            EXPECT_EQ(l1->str(), "!@#$%");
        }
    }
}


#endif