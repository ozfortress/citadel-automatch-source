#pragma once

#include <cstdint>

struct SteamID {
    uint64_t value;

    explicit SteamID(uint64_t);

    bool operator==(const SteamID) const;
    bool operator!=(const SteamID) const;

    static const SteamID null;
};
