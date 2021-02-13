#include "gtest/gtest.h"
#include "../src/impl/ref.h"

#define X

#ifdef X
static int count = 0;
struct A {
    A() {
        count++;
    }
    ~A() {
        count--;
    }
    A(A&& k) {
        count++;
    };
};
TEST(SHARED, text) {
    {
        KLib42::KShared<A> w(new A);
        ASSERT_NE(&w, nullptr);
        ASSERT_EQ(count, 1);
    }
    ASSERT_EQ(count, 0);

    {
        auto w = KLib42::makeShared(A{});
        ASSERT_EQ(count, 1);
        auto s = w.clone();
        ASSERT_EQ(count, 1);
        auto s1 = s.clone();
        ASSERT_EQ(count, 1);
    }
    ASSERT_EQ(count, 0);

    {
        auto w = KLib42::makeShared(A{});
        auto s = w.clone();
        auto s1 = s.clone();
        ASSERT_EQ(count, 1);
    }
    ASSERT_EQ(count, 0);

    {
        auto w = KLib42::makeShared(A{});
        {   
            auto s = w.clone();
            auto s1 = s.clone();
            ASSERT_EQ(count, 1);
        }
    }
};

TEST(SHARED, makeShared) {
    auto a = KLib42::makeShared(134);
    {
        *a = 100;
    }
    ASSERT_EQ(*a, 100);
    
    {
        *a += 1;
    }
    ASSERT_EQ(*a, 101);
}

TEST(SHARED, weak) {
    auto a = KLib42::makeShared(std::string("abc"));
    {
        a->push_back('1');
        ASSERT_EQ(*a, "abc1");
    }

    {
        a->push_back('2');
        ASSERT_EQ(*a, "abc12");
    }

    {
        {
            ASSERT_EQ(a.weakCount(), 1);
            auto b = a.clone();
            ASSERT_EQ(a.refCount(), 2);
        }
        
        
        {
            auto w = a.getWeak();
            ASSERT_EQ(a.weakCount(), 2);
            ASSERT_EQ(a.refCount(), 1);
            ASSERT_EQ(w.exist(), true);
            {auto _ = std::move(a); }
            ASSERT_EQ(w.exist(), false);
            ASSERT_EQ(w.weakCount(), 1);
            ASSERT_EQ(w.refCount(), 0);
            
        }
    }
}

#endif