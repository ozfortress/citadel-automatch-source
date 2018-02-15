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
    State state = State::initializing;

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
};
