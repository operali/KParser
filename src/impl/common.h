#pragma once

#include <string>
#include <vector>
#include <memory>
#include "any.h"
#include <tuple>
#include <functional>

namespace KParser {
    using StrT = std::string;
    
    using AnyT = KAny;

    template<typename T>
    using VecT = std::vector<T>;

}
    