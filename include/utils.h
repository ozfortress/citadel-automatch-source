/**
 * C++ is a heaping pile of garbage. This is a bunch of shit that should be in
 * the standard library.
 */
#pragma once

#include <memory>
#include <iostream>
#include <string>
#include <cstdio>

template<typename ... Args>
std::string format(const std::string& formatString, Args ... args) {
    auto c_formatString = formatString.c_str();
    auto size = snprintf(nullptr, 0, c_formatString, args ...) + 1;
    std::unique_ptr<char[]> buffer(new char[size]);
    snprintf(buffer.get(), size, c_formatString, args ...);

    return std::string(buffer.get(), buffer.get() + size - 1);
}

#define assert(EXPRESSION, MESSAGE) _assert(EXPRESSION, [&] () { return MESSAGE; }, __FILE__, __LINE__)

void _assert(bool expression, std::function<std::string()> lazyMessage, std::string file, int lineNo);
