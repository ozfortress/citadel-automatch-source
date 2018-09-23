#pragma once

#include "citadel/iclient.h"

namespace mocks {
    class CitadelClient : public citadel::IClient {
    public:
        CitadelClient() {}
        ~CitadelClient() {}

        void findMatchForPlayers(
            SteamID invokerSteamID,
            std::vector<SteamID> playerSteamIDs,
            std::function<void (std::vector<citadel::Match> matches)> onResult,
            ErrorCallback onError) override {}

        void registerPlugin(
            uint64_t matchId,
            SteamID starter,
            std::string address,
            std::string password,
            std::string rconPassword,
            std::vector<SteamID> team1,
            std::vector<SteamID> team2,
            std::function<void (std::string registrationToken, std::string confirmationURL)> onResult,
            ErrorCallback onError) override {}

        void registerMatch(
            uint64_t matchId,
            std::string registrationToken,
            std::function<void (std::string matchToken)> onResult,
            ErrorCallback onError) override {}

        void submitMatch(
            uint64_t matchId,
            std::string matchToken,
            MatchResult result,
            std::function<void ()> onResult,
            ErrorCallback onError) override {}
    };
}
