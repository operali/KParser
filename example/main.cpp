#ifdef _WIN32
#include <crtdbg.h>
#ifdef _DEBUG

#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

#include <iostream>
#include <string>
#include "../src/kparser.h"
using namespace KLib42;

void example_MixedOperation() {
	EasyParser p;
    p.prepareRules(R"(
op1 = '+' | '-';
op2 = '*' | '/';
operation1 = operation2 op1 operation1 | operation2;
operation2 = term op2 operation2 | term;
term = '(' operation1 ')' | NUM;
)");
    auto onOp = [&](Match& m, IT b, IT e) {
        auto c = m.str();
        return c[0];
    };
    auto onOprand = [&](Match& m, auto b, auto e) {
        if (e - b == 3) {
            double left = *b->get<double>();
            int op = *(b + 1)->get<char>();
            double right = *(b+2)->get<double>();
            if (op == '+') {
                return left + right;
            } else if (op == '-') {
                return left - right;
            } else if (op == '*') {
                return left * right;
            } else if (op == '/') {
                return left / right;
            }
        }
        else {
            return *b->get<double>();
        }
    };
    p.prepareCapture("op1", onOp);
    p.prepareCapture("op2", onOp);
    p.prepareCapture("operation1", onOprand);
    p.prepareCapture("operation2", onOprand);
    auto r = p.build();
    if (!r) {
        std::cerr << p.getLastError()->message() << std::endl;
    }
    else {
        auto* exp = "(1+1*2)*3";
        auto m = p.parse("operation1", exp);
        if (!m) {
            std::cerr << p.getLastError()->message();
        }
        else {
            std::cout << "result: " << *m->capture<int>(0);
        }
    }
}


int main() {
#ifdef _WIN32
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif
    example_MixedOperation();
#ifdef _WIN32
    _CrtDumpMemoryLeaks();
#endif
}