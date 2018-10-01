#include "utils.h"

#include <cstdio>

#include "tier2/tier2.h"

// Fucking valve decided to make these macros instead of using stdlib
#undef min
#undef max

void _sassert(bool expression, std::function<std::string()> lazyMessage, std::string file, int lineNo) {
    if (expression) return;

    auto message = format("Assertion failed at %s:%d: %s\n", file.c_str(), lineNo, lazyMessage().c_str());
    Warning("citadel-automatch: %s", message.c_str());
    std::cout << message;
    abort();
}

uint32_t levenshtein_distance(const std::string& s1, const std::string& s2) {
    const auto len1 = s1.size();
    const auto len2 = s2.size();

    std::vector<uint32_t> col(len2 + 1);
    std::vector<uint32_t> prevCol(len2 + 1);

    for (uint32_t i = 0; i < prevCol.size(); i++) {
        prevCol[i] = i;
    }

    for (uint32_t i = 0; i < len1; i++) {
        col[0] = i + 1;
        for (uint32_t j = 0; j < len2; j++) {
            // note that std::min({arg1, arg2, arg3}) works only in C++11,
            // for C++98 use std::min(std::min(arg1, arg2), arg3)
            col[j + 1] = std::min({ prevCol[1 + j] + 1, col[j] + 1, prevCol[j] + (s1[i]==s2[j] ? 0 : 1) });
        }

        col.swap(prevCol);
    }

    return prevCol[len2];
}
