#include "gtest/gtest.h"
#include "../src/impl/json.hpp"
#include "./conf.h"

#ifdef enable_test_util

using tjson = nlohmann::json;
TEST(U, T1) {
	auto j3 = tjson::parse("{ \"happy\": true, \"pi\": [3,1,4] }");
	{
		EXPECT_EQ(j3.is_object(), true);
	}

	auto items = j3.items();
	for (auto it : items) {
		std::cout << "key:" << it.key() << std::endl;
		std::cout << "value:" << it.value() << std::endl;
		auto& v = it.value();
		std::cout << "is_bool:" << v.is_boolean() << std::endl;
		std::cout << "size:" << v.size() << std::endl;
		std::cout << it << std::endl;
	}
	
}

#endif