// author: operali

#include "gtest/gtest.h"
#include "../src/detail/any.h"
#include "./conf.h"

#ifdef enable_test_any

using namespace KLib42;

TEST(ANY, t1) {
	
	{
		KAny v = (int)123;
		EXPECT_EQ(v.is<nullptr_t>(), false);
		EXPECT_EQ(v.is<int>(), true);
		EXPECT_EQ(v.is<std::string>(), false);
		EXPECT_EQ(v.toString(), "123");
	}
	

	{
		KAny v = std::string("abc");
		EXPECT_EQ(v.is<nullptr_t>(), false);
		EXPECT_EQ(v.is<int>(), false);
		EXPECT_EQ(v.is<std::string>(), true);
		EXPECT_EQ(v.toString(), "abc");
	}

	{
		KAny v;
		EXPECT_EQ(v.is<nullptr_t>(), true);
		EXPECT_EQ(v.toString(), "nullptr");
	}

	{
		KAny v = nullptr;
		EXPECT_EQ(v.is<nullptr_t>(), true);
		EXPECT_EQ(v.toString(), "nullptr");
	}

	{
		char* k = new char;
		KAny v = k;
		std::cout << v.toString() << std::endl;
		delete k;
	}
}

#endif