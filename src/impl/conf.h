// author: operali
// desc: compile configurations

#pragma once

#include <string>
#include <vector>
#include <memory>
#include "any.h"
#include <tuple>
#include <functional>

// for debuging
// #define KOBJECT_DEBUG

#define KSIZE_64BIT

#ifdef KSIZE_64BIT
#define KSIZE int64_t
#define KUSIZE uint64_t
#else
#define KSIZE int32_t
#define KUSIZE uint32_t
#endif

