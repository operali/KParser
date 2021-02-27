#include "gtest/gtest.h"
#include "../src/impl/ptree.h"
#include "./conf.h"

#ifdef enable_test_ptree

using namespace KLib42;

TEST(PTREE, t1) {
	KDocument doc;
	auto p = doc.createElement();
	p->set<int>(100);

	EXPECT_EQ(p->isElement(), true);
	EXPECT_EQ(p->isArray(), false);
	EXPECT_EQ(p->isObject(), false);

	auto p1 = doc.createArray();
	p1->addRaw(p);
	EXPECT_EQ(p1->isElement(), false);
	EXPECT_EQ(p1->isArray(), true);
	EXPECT_EQ(p1->isObject(), false);


	auto p2 = doc.createObject();
	p2->addByNameRaw("abc", p1);
	// abc.[0].[int]
	EXPECT_EQ(p2->isElement(), false);
	EXPECT_EQ(p2->isArray(), false);
	EXPECT_EQ(p2->isObject(), true);

	EXPECT_EQ(p2->getByNameRaw("cde"), nullptr);
	{
		KProperty* pp = p2->getByNameRaw("abc");
		ASSERT_TRUE(pp);
		auto pp1 = pp->getByIndexRaw(0);
		ASSERT_TRUE(pp1);
		auto v = pp1->get<int>();
		ASSERT_TRUE(v != nullptr);
		EXPECT_EQ(*v, 100);
	}

	{
		KProperty* pp = p2->getByNameRaw("abc");
		ASSERT_TRUE(pp);
		auto pint = doc.createElement();
		pint->set(std::string("1234"));
		pp->addRaw(pint);
		pp = pp->getByIndexRaw(1);
		auto v = pp->get<std::string>();
		ASSERT_TRUE(v != nullptr);
		EXPECT_EQ(*v, "1234");
	}

	{
		auto* elem = doc.createElement(std::string("abcde"));
		ASSERT_TRUE(elem != nullptr);
		EXPECT_EQ(elem->get<int>(), nullptr);
		EXPECT_EQ(*elem->get<std::string>(), "abcde");
		
		auto* elem1 = doc.createArray()->add(doc, (int)1234)->add(doc, (float)33.3);
		EXPECT_EQ(*elem1->getByIndex<int>(0), 1234);
		EXPECT_FLOAT_EQ(*elem1->getByIndex<float>(1), 33.3);
	}
}

#endif
