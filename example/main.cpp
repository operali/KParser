#ifdef _WIN32
#include <crtdbg.h>
#ifdef _DEBUG

#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

#include <iostream>
#include "../src/kparser.h"

void example() {
    KLib42::EasyParser p;
    std::string strval = "";
    p.prepareRules(R"(
op1 = '+' | '-';
op2 = '*' | '/';
exp1 = [term op1];
exp2 = [exp1 op2];
exp = exp2;
term = NUM | '(' exp ')';
)");
    p.prepareCapture("exp", [&](auto& m, auto argB, auto argE) {
        std::cout << "exp" << std::endl;
        return nullptr;
    });
    auto r = p.build();
    if (!r) {
        std::cerr << p.getLastError()->message() << std::endl;
    }
    else {
        char* k = new char;
        p.parse("exp", "1+1");
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