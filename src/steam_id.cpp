#include "steam_id.h"

const SteamID SteamID::null = SteamID(0);

SteamID::SteamID(uint64_t value) : value(value) {}

bool SteamID::operator==(const SteamID other) const {
    return value == other.value;
}

bool SteamID::operator!=(const SteamID other) const {
    return value != other.value;
}
