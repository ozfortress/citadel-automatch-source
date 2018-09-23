/**
 * C++ is a heaping pile of garbage. This is a bunch of shit that should be in
 * the standard library.
 */
#pragma once

#include <memory>
#include <iostream>
#include <string>
#include <cstdio>
#include <functional>

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

// trim from start (in place)
static inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string& s) {
    ltrim(s);
    rtrim(s);
}

uint32_t levenshtein_distance(const std::string& s1, const std::string& s2);

// Some stupid shit to make std::visit usable
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
