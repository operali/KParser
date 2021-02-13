#include "gtest/gtest.h"

#define X

#ifdef X

#include "../src/impl/any.h"


TEST(UTIL, KAny) {
    using NINT = int;
    using namespace KLib42;
    {
        int a = 1234;
        KAny va(a);

        KAny vb(std::string("asdf"));

        EXPECT_EQ(va.is<std::string>(), false);
        EXPECT_EQ(va.is<int>(), true);
        EXPECT_EQ(*va.get<int>(), 1234);
        EXPECT_EQ(va.get<std::string>(), nullptr);

        EXPECT_EQ(vb.is<std::string>(), true);
        EXPECT_EQ(vb.is<int>(), false);
        EXPECT_EQ(vb.get<int>(), nullptr);
        EXPECT_EQ(*vb.get<std::string>(), "asdf");
    }

    {
        static int count = 0;
        struct A {
            A() {
                count++;
            }
            A(const A& a) noexcept {
                val = a.val;
                count++;
            }
            ~A() {
                count--;
            }
            int val = 0;

        };

        {
            KAny va(A{});
            EXPECT_EQ(count, 1);
        }
        EXPECT_EQ(count, 0);
        
        {
            KAny va(A{});
            EXPECT_EQ(count, 1);
            
            auto* a = va.get<A>();
            a->val = 100;
            EXPECT_EQ(va.get<A>()->val, 100);
        }
        EXPECT_EQ(count, 0);
    }
    
}

#endif