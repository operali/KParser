#pragma once

#include <string>
#include <vector>
#include <memory>
#include <any>
#include <tuple>
#include <variant>
#include <functional>

namespace KParser {
    using StrT = std::string;
    
    using AnyT = std::any;

    template<typename T>
    using VecT = std::vector<T>;

    using PredT = std::function<const char*(const char* start, const char* end, AnyT& val)>;

    // predeclare
    struct CLSINFO {
        virtual StrT getName() = 0;
    };
}
    