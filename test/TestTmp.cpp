// author: operali

#include "gtest/gtest.h"

#include "../src/kparser.h"
#include "../src/impl/impl.h"
#include "../src/impl/error.h"

#include "../src/impl/dsl.h"

#include "./conf.h"

#ifdef enable_test_tmp

//TEST(BASIC, trace) {
//	KLib42::Parser p;
//	auto r1 = p.str("1234");
//	r1->setName("r_1234");
//	auto r2 = p.str("abcd");
//	r2->setName("r_abcd");
//	auto r = p.any(r1, r2);
//	r->setName("any");
//
//	auto rr = p.all(r, r);
//	rr->setName("root");
//    p.enableTrace(true);
//	auto m = rr->parse("1234 abcd");
//    ASSERT_EQ(p.getDebugInfo(), "on_rule: root\n on_rule: any\n  on_rule: r_1234\n   succ\n  succ\n on_rule: any\n  on_rule: r_1234\n   fail\n  on_rule: r_abcd\n   succ\n  succ\n");
//}


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