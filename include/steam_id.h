#pragma once

#include <cstdint>

struct SteamID {
    uint64_t value;

    explicit SteamID(uint64_t);

    SteamID(const SteamID &o) : value(o.value) {}

    bool operator==(const SteamID) const;
    bool operator!=(const SteamID) const;
};
