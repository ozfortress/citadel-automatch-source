#pragma once

#include <vector>
#include <string>

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

public:
    Match();
    ~Match();

    void log(std::string);

    std::string getLogs();
};
