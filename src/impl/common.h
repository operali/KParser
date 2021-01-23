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

}
    