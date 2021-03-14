// author: operali

#include "gtest/gtest.h"
#include "./conf.h"
#include <sstream>
#include "../src/detail/util.h"


#ifdef enable_test_util



TEST(UTIL, parseFloat) {
	char* str = "";
	double retV;
	int retL = 0;
	bool r = KLib42::parseFloat(str, strlen(str), retV, retL);
	ASSERT_EQ(r, false);

	str = "a";
	r = KLib42::parseFloat(str, strlen(str), retV, retL);
	ASSERT_EQ(r, false);

	str = ".";
	r = KLib42::parseFloat(str, strlen(str), retV, retL);
	ASSERT_EQ(r, false);

	str = "0";
	r = KLib42::parseFloat(str, strlen(str), retV, retL);
	ASSERT_EQ(r, true);
	EXPECT_EQ(retV, 0);
	EXPECT_EQ(retL, 1);

	str = "+0";
	r = KLib42::parseFloat(str, strlen(str), retV, retL);
	ASSERT_EQ(r, true);
	EXPECT_EQ(retV, 0);
	EXPECT_EQ(retL, 2);

	str = "-0";
	r = KLib42::parseFloat(str, strlen(str), retV, retL);
	ASSERT_EQ(r, true);
	EXPECT_EQ(retV, 0);
	EXPECT_EQ(retL, 2);

	str = ".1";
	r = KLib42::parseFloat(str, strlen(str), retV, retL);
	ASSERT_EQ(r, true);
	
	str = "-.1";
	r = KLib42::parseFloat(str, strlen(str), retV, retL);
	ASSERT_EQ(r, true);

	str = "0.1";
	r = KLib42::parseFloat(str, strlen(str), retV, retL);
	ASSERT_EQ(r, true);
	EXPECT_DOUBLE_EQ(retV, 0.1);
	EXPECT_EQ(retL, 3);


	str = "1234.567890";
	r = KLib42::parseFloat(str, strlen(str), retV, retL);
	ASSERT_EQ(r, true);
	EXPECT_DOUBLE_EQ(retV, 1234.567890);
	EXPECT_EQ(retL, 11);

	double a = 1000.0E32 / 1200.3333E44;
}

TEST(UTIL, parseInteger) {
	char* str = "";
	int64_t retV;
	int retL = 0;
	bool r = KLib42::parseInteger(str, strlen(str), retV, retL);
	ASSERT_EQ(r, false);

	str = "a";
	r = KLib42::parseInteger(str, strlen(str), retV, retL);
	ASSERT_EQ(r, false);

	str = "0";
	r = KLib42::parseInteger(str, strlen(str), retV, retL);
	ASSERT_EQ(r, true);
	EXPECT_EQ(retV, 0);
	EXPECT_EQ(retL, 1);

	str = "+0";
	r = KLib42::parseInteger(str, strlen(str), retV, retL);
	ASSERT_EQ(r, true);
	EXPECT_EQ(retV, 0);
	EXPECT_EQ(retL, 2);

	str = "-0";
	r = KLib42::parseInteger(str, strlen(str), retV, retL);
	ASSERT_EQ(r, true);
	EXPECT_EQ(retV, 0);
	EXPECT_EQ(retL, 2);

	str = ".1";
	r = KLib42::parseInteger(str, strlen(str), retV, retL);
	ASSERT_EQ(r, false);

	str = "-.1";
	r = KLib42::parseInteger(str, strlen(str), retV, retL);
	ASSERT_EQ(r, false);

	str = "0.1";
	r = KLib42::parseInteger(str, strlen(str), retV, retL);
	ASSERT_EQ(r, true);
	EXPECT_EQ(retV, 0);
	EXPECT_EQ(retL, 1);


	str = "1234.567890";
	r = KLib42::parseInteger(str, strlen(str), retV, retL);
	ASSERT_EQ(r, true);
	EXPECT_EQ(retV, 1234);
	EXPECT_EQ(retL, 4);
}

TEST(UTIL, parseIdentifier) {
	{
		const char* str = "";
		int retL = 0;
		bool r = KLib42::parseIdentifier(str, strlen(str), retL);
		ASSERT_EQ(r, false);
	}
	{
		const char* str = "0";
		int retL = 0;
		bool r = KLib42::parseIdentifier(str, strlen(str), retL);
		ASSERT_EQ(r, false);

		str = "9";
		r = KLib42::parseIdentifier(str, strlen(str), retL);
		ASSERT_EQ(r, false);
	}
	{
		const char* str = "_";
		int retL = 0;
		bool r = KLib42::parseIdentifier(str, strlen(str), retL);
		ASSERT_EQ(r, true);
		EXPECT_EQ(retL, 1);
	}
	{
		const char* str = "aAZz_";
		int retL = 0;
		bool r = KLib42::parseIdentifier(str, strlen(str), retL);
		ASSERT_EQ(r, true);
		EXPECT_EQ(retL, 5);
	}
	{
		const char* str = "__";
		int retL = 0;
		bool r = KLib42::parseIdentifier(str, strlen(str), retL);
		ASSERT_EQ(r, true);
		EXPECT_EQ(retL, 2);
	}
	{
		const char* str = "_9";
		int retL = 0;
		bool r = KLib42::parseIdentifier(str, strlen(str), retL);
		ASSERT_EQ(r, true);
		EXPECT_EQ(retL, 2);
	}
	{
		const char* str = "a";
		int retL = 0;
		bool r = KLib42::parseIdentifier(str, strlen(str), retL);
		ASSERT_EQ(r, true);
		EXPECT_EQ(retL, 1);
	}
	{
		const char* str = "a,b";
		int retL = 0;
		bool r = KLib42::parseIdentifier(str, strlen(str), retL);
		ASSERT_EQ(r, true);
		EXPECT_EQ(retL, 1);
	}
	{
		const char* str = "aa,";
		int retL = 0;
		bool r = KLib42::parseIdentifier(str, strlen(str), retL);
		ASSERT_EQ(r, true);
		EXPECT_EQ(retL, 2);
	}
}
#endif