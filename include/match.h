#pragma once

#include <vector>
#include <string>
#include <variant>

#include "igame.h"
#include "citadel/iclient.h"

class Match {
public:
    struct Initializing {
    };

    struct ConfirmationPending {
        bool team1Confirmed;
        bool team2Confirmed;
    };

    struct Running {

    };

    using State = std::variant<
        Initializing,
        ConfirmationPending,
        Running
    >;

private:
    State state = Initializing();

    std::vector<std::string> logs;

    std::shared_ptr<IGame> game;
    std::shared_ptr<citadel::IClient> citadel;
    uint64_t matchId;

    void log(std::string);
    std::string getLogs();

public:
    Match(std::shared_ptr<IGame>, std::shared_ptr<citadel::IClient>, uint64_t matchId);
    ~Match();

    void start();

    bool onCommand(std::string, SteamID, Team);
};
