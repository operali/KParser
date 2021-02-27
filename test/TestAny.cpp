#include "gtest/gtest.h"
#include "../src/impl/any.h"
#include "./conf.h"

#ifdef enable_test_any

using namespace KLib42;

TEST(ANY, t1) {
	
	{
		KAny v = 123;
		EXPECT_EQ(v.toString(), "123");
	}
	

	{
		// NOTE: 
		KAny v = std::string("abc");
		EXPECT_EQ(v.toString(), "abc");
	}

	{
		KAny v;
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