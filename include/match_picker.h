#pragma once

#include "citadel/iclient.h"
#include "match.h"
#include "requests.h"
#include "igame.h"

class MatchPicker {
    struct Match {
        std::shared_ptr<citadel::IClient> client;
        citadel::Match details;

        Match(std::shared_ptr<citadel::IClient> c, citadel::Match m) : client(c), details(m) {}
    };

    SteamID starter = SteamID(0);
    std::vector<std::shared_ptr<citadel::IClient>> clients;
    size_t clientResults;
    std::vector<Match> matches;

    void afterAllResults();

public:
    MatchPicker(const std::vector<std::shared_ptr<citadel::IClient>> &c) : clients(c) {}

    template <class C>
    static std::unique_ptr<MatchPicker> create(std::shared_ptr<Requests> requests, std::vector<std::string> endpoints) {
        std::vector<std::shared_ptr<citadel::IClient>> clients;

        for (auto &endpoint : endpoints) {
            clients.push_back(std::make_unique<C>(requests, endpoint));
        }

        return std::make_unique<MatchPicker>(clients);
    }

    void queryAll(SteamID starter, std::vector<SteamID> &players);

    std::unique_ptr<::Match> onCommand(std::string, SteamID, Team);
};
