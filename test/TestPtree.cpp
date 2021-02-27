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
	p1->setRaw(p);
	EXPECT_EQ(p1->isElement(), false);
	EXPECT_EQ(p1->isArray(), true);
	EXPECT_EQ(p1->isObject(), false);


	auto p2 = doc.createObject();
	p2->setByNameRaw("abc", p1);
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
		pp->setRaw(pint);
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

		auto* obj = doc.createObject()->addByName(doc, "abc", (int)1234)->addByName(doc, "abc1", std::string("asdf") );
		EXPECT_EQ(obj->getByName<float>("abc"), nullptr);
		EXPECT_EQ(*obj->getByName<int>("abc"), 1234);
	}
}


TEST(PTREE, t2) {
	KDocument doc;
	
	{
		auto p = doc.createObject();
		auto p1 = p->setByPath(doc, "a", (std::string("abcd")));
		ASSERT_EQ(p->getByPathRaw("a") != nullptr, true);
		auto p2 = p->getByPathRaw("a");
		EXPECT_EQ(p1, p2);

		std::string v = *p->getByPath<std::string>("a");
		EXPECT_EQ(v, "abcd");
	}

	{
		auto p = doc.createObject();
		auto p1 = p->setByPath(doc, "a/b/c", (int)3);
		ASSERT_EQ(p->getByPathRaw("a") != nullptr, true);
		ASSERT_EQ(p->getByPathRaw("a/b") != nullptr, true);
		auto p2 = p->getByPathRaw("a/b/c");
		EXPECT_EQ(p1, p2);

		int v = *p->getByPath<int>("a/b/c");
		EXPECT_EQ(v, 3);
	}

}
#endif
