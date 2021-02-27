#ifdef _WIN32
#include <crtdbg.h>
#ifdef _DEBUG

#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

#include <iostream>
#include "../src/kparser.h"

void example() {
    printf("demo");
	KLib42::EasyParser p;
    std::string strval = "";
    p.prepareRules(R"(
a = ID | NUM;
b = (...a)* EOF;
)");
    p.prepareCapture("a", [&](KLib42::Match& m, KLib42::IT arg, KLib42::IT noarg) {
        int* pNum = arg->get<int>();
        if (pNum) {
            int num = *pNum;
            std::cout << "number of" << num << std::endl;
            return nullptr;
        }
        else {
            std::string* pid = arg->get<std::string>();
            if (pid) {
                auto id = *pid;
                std::cout << "string of" << id << std::endl;
                return nullptr;
            }
            else {
                return nullptr;
            }

        }
        });
    auto r = p.build();
    if (!r) {
        std::cerr << p.getLastError()->message() << std::endl;
    }
    else {
        char* k = new char;
        p.parse("b", "11, 22 ,aa, bb");
    }

}


int main() {
#ifdef _WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif
    example();
#ifdef _WIN32
    _CrtDumpMemoryLeaks();
#endif
}