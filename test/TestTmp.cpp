#include "gtest/gtest.h"

#include "../src/KParser.h"
#include "../src/impl/impl.h"
#include "../src/impl/error.h"

TEST(TEXT, TMP1) {
	using namespace KLib42;
	{
		EasyParser e;
		{
			e.prepareRules(R"(
1234
asdf
dddd
)");
			
		}
	}
	

}