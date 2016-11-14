#include <iostream>
#include <vector>
#include <tuple>
#include <chrono>
#include <unordered_map>
#include <vector>
#include <map.h>
#include <algorithm>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <regex>
#include <random>
#include <vectorization.h>
#include "stream/stream.h"

/*
 * Feetchury
 * Quality of life: auto, decltype, foreach, init lists, delegating constructors, default/delete, final, override, nullptr, enum class, variadic templates, varargs, tuples
 * lambdy, capture, std::function, std::bind, partial application
 * r-value/move, emplace_back, rule of three (five)
 * RAII pointers
 * stdlib: thread, atomic, sync primitives, async/future, regex, random numbers
 * vectorization
 * Proof of concept: stream
 * AddressSanitizer
 * ~
 * */

auto recurse(int x) -> int
{
    if (x <= 2) return recurse(x - 1);
    return 0;
}

int main(int argc, char** argv)
{
    std::cout << recurse(5) << std::endl;

    return 0;
}
