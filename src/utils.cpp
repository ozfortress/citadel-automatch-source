#include "utils.h"

#include <cstdio>

#include "tier2/tier2.h"

void _assert(bool expression, std::function<std::string()> lazyMessage, std::string file, int lineNo) {
    if (expression) return;

    auto message = format("Assertion failed at %s:%d: %s\n", file.c_str(), lineNo, lazyMessage().c_str());
    Warning("citadel-automatch: %s", message.c_str());
    std::cout << message;
    abort();
}
