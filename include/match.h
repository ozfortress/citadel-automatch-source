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

        ConfirmationPending(std::string a, std::string b)
                : registrationToken(a)
                , confirmationURL(b) {}
    };

    struct Running {
        std::string matchToken;

        Running(std::string m) : matchToken(m) {}
    };

    using State = std::variant<
        Initializing,
        ConfirmationPending,
        Running
    >;

private:
    std::vector<std::string> logs;

    IGame *game;
    std::shared_ptr<citadel::IClient> citadel;
    citadel::Match matchInfo;

    void log(std::string);
    std::string getLogs();

    void onPlayerConfirm(std::string &, IPlayer *);

    void onMatchComplete(uint32_t homeTeamScore, uint32_t awayTeamScore);

public:
    State state = Initializing();

    Match(IGame *, std::shared_ptr<citadel::IClient>, const citadel::Match& match);
    ~Match();

    void start(IPlayer *starter);

    bool onCommand(IPlayer *, std::string);

    void onServerConfirm();
    void onServerConfirmationProgress();
};
