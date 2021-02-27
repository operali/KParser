#ifdef _WIN32
#include <crtdbg.h>
#ifdef _DEBUG

#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

#include <iostream>
#include <string>
#include "../src/kparser.h"

struct EVAL {
    std::vector<char> ops;
    std::vector<float> oprs;
    void eval() {
        auto c = ops.back();
        ops.pop_back();

        auto opr1 = oprs.back();
        auto opr2 = oprs.back();
        
        if (c == '+') {
            auto res = opr1 + opr2;
            oprs.push_back(res);
        }
        if (c == '-') {
            auto res =  opr2 - opr1;
            oprs.push_back(res);
        }
        if (c == '*') {
            auto res = opr1 * opr2;
            oprs.push_back(res);
        }
        if (c == '/') {
            auto res = opr2 / opr1;
            oprs.push_back(res);
        }
    }

};


void example() {
    printf("demo");
	KLib42::EasyParser p;
    std::string strval = ""; 
    EVAL ev;
    p.prepareRules(R"(
op1 = '+' | '-';
op2 = '*' | '/';
operation1 = operation2 op1 operation1 | operation2;
operation2 = term op2 operation2 | term;
term = '(' term ')' | NUM;
)");
    
    p.prepareCapture("op1", [&](KLib42::Match& m, auto b, auto e) {
        char c = m.str()[0];
        ev.ops.push_back(c);
        return c;
        });
    p.prepareCapture("op2", [&](KLib42::Match& m, auto b, auto e) {
        char c = m.str()[0];
        ev.ops.push_back(c);
        return c;
        });
    p.prepareCapture("operation1", [&](KLib42::Match& m, auto b, auto e) {
        while (b != e) {
            auto& v = (*b);
            int* opr = (*b++).get<int>();
            if (opr) {
                ev.oprs.push_back(*opr);
            }
            else {
                ev.eval();
            }
        }        
        return nullptr;
    });
    p.prepareCapture("operation2", [&](KLib42::Match& m, KLib42::IT b, KLib42::IT e) {
        while (b != e) {
            auto& v = (*b);
            int* opr = (*b++).get<int>();
            if (opr) {
                ev.oprs.push_back(*opr);
            }
            else {
                ev.eval();
            }
        }
        return nullptr;
        });
    auto r = p.build();
    if (!r) {
        std::cerr << p.getLastError()->message() << std::endl;
    }
    else {
        char* k = new char;
        auto m = p.parse("operation1", "1+1");
        if (!m) {
            std::cerr << p.getLastError()->message();
        }
        else {
            std::cout << ev.oprs.back();
        }
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