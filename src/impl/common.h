#pragma once

#include <string>
#include <vector>
#include <memory>
#include "any.h"
#include <tuple>
#include <functional>

namespace KParser {
    using StrT = std::string;
    
    using AnyT = libany::any;

    template<typename T>
    using VecT = std::vector<T>;

    using PredT = std::function<void(const char* begin, const char* textEnd, const char*& matchBegin, const char*& matchEnd, const char*& end)>;

    // predeclare
    struct CLSINFO {
        virtual StrT getName() = 0;
    };
}
    