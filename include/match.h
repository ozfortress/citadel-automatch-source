#pragma once

#include <vector>
#include <string>
#include <variant>

#include "igame.h"
#include "citadel/iclient.h"

class Match final {
public:
    struct Initializing {
    };

    struct ConfirmationPending {
        std::string registrationToken;
        std::string confirmationURL;

        // bool team1Confirmed;
        // bool team2Confirmed;

        ConfirmationPending(std::string a, std::string b) : registrationToken(a), confirmationURL(b) {}
    };

    struct Running {

    };

    using State = std::variant<
        Initializing,
        ConfirmationPending,
        Running
    >;

private:
    std::vector<std::string> logs;

    std::shared_ptr<IGame> game;
    std::shared_ptr<citadel::IClient> citadel;
    uint64_t matchId;

    void log(std::string);
    std::string getLogs();

    void onPlayerConfirm(ConfirmationPending&, SteamID, Team);

public:
    State state = Initializing();

    Match(std::shared_ptr<IGame>, std::shared_ptr<citadel::IClient>, uint64_t matchId);
    ~Match();

    void start();

    bool onCommand(std::string, SteamID, Team);

    void onServerConfirm();
};
