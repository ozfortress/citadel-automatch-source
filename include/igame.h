#pragma once

#include <string>
#include <vector>

#include "steam_id.h"

// TODO
enum class ChatColor {
    white = '\x01',
    darkRed = '\x02',
    // white2 = '\x03'
    darkGreen = '\x04',
    mossGreen = '\x05',
    limeGreen = '\x06',
    lightRed = '\x07',
    // lightRed2 = '\x08',
    // lightRed3 = '\x09',
};

enum class Team {
    other = 1,
    team1 = 2,
    team2 = 3,
};

class IPlayer {
public:
    virtual const std::string_view getName() const = 0;
    virtual void setName(const std::string_view) = 0;
    virtual SteamID getSteamID() const = 0;
    virtual Team getTeam() const = 0;

    virtual void notify(const std::string_view) = 0;
    virtual void notifyError(const std::string_view) = 0;

    virtual void openMOTD(const std::string_view title, const std::string_view url) = 0;
};

class IGame {
public:
    virtual int serverPort() const = 0;
    virtual const std::string_view serverPassword() const = 0;
    virtual const std::string_view serverRConPassword() const = 0;
    virtual const std::string_view serverName() const = 0;
    virtual std::vector<IPlayer *> team1Players() const = 0;
    virtual std::vector<IPlayer *> team2Players() const = 0;
    virtual std::vector<IPlayer *> nonTeamPlayers() const = 0;

    virtual void resetMatch() = 0;

    virtual void notifyAll(const std::string_view) = 0;
    virtual void notifyAllError(const std::string_view) = 0;
};
