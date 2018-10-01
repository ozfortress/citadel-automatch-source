#pragma once

#include "citadel/iclient.h"
#include "match.h"
#include "igame.h"
#include "requests.h"
#include "igame.h"

class MatchPicker {
    struct Match {
        const std::shared_ptr<citadel::IClient> client;
        const citadel::Match details;

        Match(const std::shared_ptr<citadel::IClient> c, const citadel::Match& m) : client(c), details(m) {}
    };

    IPlayer *starter;
    IGame *game;
    const std::vector<std::shared_ptr<citadel::IClient>> clients;

    size_t clientResults = 0;
    std::vector<Match> matches;

    void afterAllResults();

public:
    MatchPicker(IPlayer *, IGame *, const std::vector<std::shared_ptr<citadel::IClient>>&);

    template <class C>
    static std::unique_ptr<MatchPicker> create(
            const IPlayer *starter,
            std::shared_ptr<Requests> requests,
            std::vector<std::string> endpoints,
            IGame *game) {
        std::vector<std::shared_ptr<citadel::IClient>> clients;

        for (auto& endpoint : endpoints) {
            clients.push_back(std::make_unique<C>(requests, endpoint));
        }

        return std::make_unique<MatchPicker>(starter, game, clients);
    }

    void queryAll(std::vector<IPlayer *>& players);

    std::unique_ptr<::Match> onCommand(IPlayer *, std::string);
};
