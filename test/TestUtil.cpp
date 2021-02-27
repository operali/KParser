#include "gtest/gtest.h"
#include "./conf.h"
#include "../src/impl/terminal.h"
#include <sstream>
#ifdef enable_test_util

#include <Windows.h>
TEST(UTIL, t1) {
	colorprintf(0, "asdf");
	std::stringstream ss;
	ss << "-----------------" << std::endl;
ss << "10 seconds have passed";
ss << "\033[B" << "11";
ss << "\033[A" << "\b" << "dddddddd";
ss << "\033[B";
ss << std::endl;
ss << "-----------------";
ss << std::endl;
ss << std::endl;
std::cout << ss.str();
}

#endif