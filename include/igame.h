#pragma once

#include <string>
#include <vector>

#include "steam_id.h"

class IGame {
public:
    virtual std::string serverAddress() = 0;
    virtual std::string serverPassword() = 0;
    virtual std::string serverRConPassword() = 0;
    virtual std::vector<SteamID> team1Players() = 0;
    virtual std::vector<SteamID> team2Players() = 0;
};
