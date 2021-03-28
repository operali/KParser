// author: operali

#include "gtest/gtest.h"

#include "../src/kparser.h"
#include "../src/detail/impl.h"
#include "../src/detail/error.h"

#include "../src/detail/dsl.h"

#include "./conf.h"

#ifdef enable_test_tmp


TEST(TEXT, TMP2) {
	
	KLib42::DSLContext ctx;
	ctx.m_ruleParser.enableTrace(true);
	{
		auto m = ctx.r_ruleList->parse(R"(c = c | b/*
)");
		std::cerr << ctx.m_ruleParser.getDebugInfo();
		//ASSERT_EQ(m.get() == nullptr, true);
		//ASSERT_EQ(!!err, true);
		std::cerr << ctx.m_ruleParser.getLastError()->message() << std::endl;
	}

	{
		auto m = ctx.r_ruleList->parse(R"(c = c | b^/d*/
)");
		ASSERT_EQ(m.get() == nullptr, true);
		
		std::cerr << ctx.getLastError()->message() << std::endl;
	}
}

#endif // enable_test_tmp