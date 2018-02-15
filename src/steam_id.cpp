#include "steam_id.h"

SteamID::SteamID(uint64_t value) : value(value) {}

bool SteamID::operator==(const SteamID other) const {
    return value == other.value;
}
