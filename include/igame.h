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

    virtual void notifyError(std::string message, SteamID target = SteamID::null) = 0;
    virtual void resetMatch() = 0;

    virtual void notifyAll(std::string) = 0;
    virtual void notify(SteamID, std::string) = 0;

    virtual void openMOTD(SteamID player, std::string title, std::string url) = 0;
};

enum class Team {
    team1 = 0,
    team2 = 1,
};
