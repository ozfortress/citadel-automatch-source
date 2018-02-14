#pragma once

#include <vector>
#include <string>

#include "igame.h"
#include "citadel/iclient.h"

class Match {
public:
    enum class State {
        initializing,
        waitingForConfirmation,
        running,
        sendingResults
    };

private:
    State state;
    std::vector<std::string> logs;

    std::shared_ptr<IGame> game;
    std::shared_ptr<citadel::IClient> citadel;

public:

    Match(std::shared_ptr<IGame>, std::shared_ptr<citadel::IClient>);
    ~Match();

    void log(std::string);
    std::string getLogs();

    void start(SteamID initiator);
};
