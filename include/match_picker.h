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

    const SteamID starter;
    const std::vector<std::shared_ptr<citadel::IClient>> clients;
    const std::shared_ptr<IGame> game;

    size_t clientResults = 0;
    std::vector<Match> matches;

    void afterAllResults();

public:
    MatchPicker(const SteamID s, const std::vector<std::shared_ptr<citadel::IClient>>& c, const std::shared_ptr<IGame> g);

    template <class C>
    static std::unique_ptr<MatchPicker> create(
            const SteamID starter,
            std::shared_ptr<Requests> requests,
            std::vector<std::string> endpoints,
            const std::shared_ptr<IGame> game) {
        std::vector<std::shared_ptr<citadel::IClient>> clients;

        for (auto& endpoint : endpoints) {
            clients.push_back(std::make_unique<C>(requests, endpoint));
        }

        return std::make_unique<MatchPicker>(starter, clients, std::move(game));
    }

    void queryAll(std::vector<SteamID>& players);

    std::unique_ptr<::Match> onCommand(SteamID, std::string);
};
