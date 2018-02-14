#pragma once

#include <cstdint>

struct SteamID {
    uint64_t value;

    explicit SteamID(uint64_t);
};
