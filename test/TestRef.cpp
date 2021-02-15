#include "gtest/gtest.h"
#include "../src/impl/common.h"

#define X

#ifdef X
static int xcount = 0;
struct A {
    A() {
        xcount++;
    }
    ~A() {
        xcount--;
    }
    A(A&& k) {
        xcount++;
    };
};

TEST(UNIQUE, test1) {
    {
        KLib42::KUnique<A> u;
        
        ASSERT_EQ(u.get(), nullptr);
        ASSERT_FALSE(u);
        ASSERT_EQ(xcount, 0);
    }
    
    {
        auto u = KLib42::makeUnique(A{});
        ASSERT_EQ(u.get() == nullptr, false);
        ASSERT_TRUE(u);
        ASSERT_EQ(xcount, 1);
    }
    ASSERT_EQ(KLib42::KObject::count, 0);

    {
        auto u = KLib42::makeUnique(A{});
        ASSERT_NE(u.get(), nullptr);
        ASSERT_TRUE(u);
        ASSERT_EQ(xcount, 1);

        KLib42::KUnique<A> k(std::move(u));
        ASSERT_NE(k.get(), nullptr);
        ASSERT_TRUE(k);
        ASSERT_EQ(xcount, 1);
        ASSERT_EQ(u.get(), nullptr);
        ASSERT_FALSE(u);
    }
    ASSERT_EQ(xcount, 0);

}


TEST(SHARED, text) {
    {
        auto w = KLib42::KShared<A>(new A{});
        {
            auto s = KLib42::KShared<A>();
            s = w;
            ASSERT_EQ(s.refCount(), 2);
            ASSERT_EQ(w.refCount(), 2);
            ASSERT_EQ(s.weakCount(), 1);
            ASSERT_EQ(w.weakCount(), 1);
        }
        ASSERT_EQ(xcount, 1);
    }
    ASSERT_EQ(xcount, 0);

    {
        KLib42::KShared<A> w(new A);
        ASSERT_NE(&w, nullptr);
        ASSERT_EQ(xcount, 1);
    }
    ASSERT_EQ(xcount, 0);

    {
        auto w = KLib42::makeShared(A{});
        ASSERT_EQ(xcount, 1);
        auto s = w.clone();
        ASSERT_EQ(xcount, 1);
        auto s1 = s.clone();
        ASSERT_EQ(xcount, 1);
    }
    ASSERT_EQ(xcount, 0);

    {
        auto w = KLib42::makeShared(A{});
        auto s = w.clone();
        auto s1 = s.clone();
        ASSERT_EQ(xcount, 1);
    }
    ASSERT_EQ(xcount, 0);

    {
        auto w = KLib42::makeShared(A{});
        {   
            auto s = w.clone();
            auto s1 = s.clone();
            ASSERT_EQ(xcount, 1);
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
            ASSERT_EQ(!!w, true);
            {auto _ = std::move(a); }
            ASSERT_EQ(!!w, false);
            ASSERT_EQ(w.weakCount(), 1);
            ASSERT_EQ(w.refCount(), 0);
            
        }
    }
}

#endif