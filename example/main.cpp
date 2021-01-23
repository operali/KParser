#include "../src/KParser.h"
#include "../src/impl/rule.h"
#include <sstream>
#include <Windows.h>
#include <exception>
#include <iostream>

int main() {
    try
    {
        {
            auto text = "abc";
            std::stringstream ss;
            ss << text;
            for (auto i = 0; i < 655; ++i) {
                ss << ", " << text;
            }

            {
                KParser::Parser p;
                int i = 0;
                auto f = [&](auto* m) {
                    i++;
                };
                auto m = p.list("abc")->on(f), ","));

                m->match(ss.str());
            }
            // EXPECT_EQ(i, 656);
        }
        std::cout << KParser::KObject::count << std::endl;
    }
    catch (std::exception& ex)
    {
        printf("Executing SEH __except block\r\n");
    }

}
