#pragma once

#include "citadel/iclient.h"

namespace mocks {
    class CitadelClient : public citadel::IClient {
    public:
        CitadelClient();
        ~CitadelClient();

        void findMatchForPlayers(std::unique_ptr<Callback<MatchForPlayersResponse>>, SteamID invokerSteamID, std::vector<SteamID> playerSteamIDs) override;

        void registerPlugin(std::unique_ptr<Callback<RegisterPluginResponse>>, uint64_t matchId, std::string address, std::string password, std::string rconPassword, std::vector<SteamID> team1, std::vector<SteamID> team2) override;

        void registerMatch(std::unique_ptr<Callback<RegisterMatchResponse>>, uint64_t matchId, std::string registrationToken) override;

        void submitMatch(std::unique_ptr<Callback<void>>, uint64_t matchId, std::string matchToken, MatchResult result) override;
    };
}
