#pragma once

#include "citadel/iclient.h"

namespace mocks {
    class CitadelClient : public citadel::IClient {
    public:
        CitadelClient();
        ~CitadelClient();

        void findMatchForPlayers(Callback<std::vector<citadel::Match>>, SteamID invokerSteamID, std::vector<SteamID> playerSteamIDs) override;

        void registerPlugin(Callback<RegisterPluginResponse>, uint64_t matchId) override;

        void registerMatch(Callback<RegisterMatchResponse>, uint64_t matchId, std::string registrationToken) override;

        void submitMatch(Callback<>, uint64_t matchId, std::string matchToken, MatchResult result) override;
    };
}
