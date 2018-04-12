#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

#include "steam_id.h"

namespace citadel {
    struct Roster {
        uint64_t id;
        std::string name;
    };

    struct Match {
        uint64_t id;
        Roster homeTeam;
        Roster awayTeam;
    };

    class IClient {
    public:
        template<class Result>
        class Callback {
        public:
            virtual void onResult(std::unique_ptr<Result>) = 0;
            virtual void onError(int32_t code, std::string error) = 0;
        };

        struct MatchForPlayersResponse {
            std::vector<Match> matches;
        };

        virtual void findMatchForPlayers(std::unique_ptr<Callback<MatchForPlayersResponse>>, SteamID invokerSteamID, std::vector<SteamID> playerSteamIDs) = 0;

        struct RegisterPluginResponse {
            std::string registrationToken;
            std::string confirmationURL;
        };

        virtual void registerPlugin(std::unique_ptr<Callback<RegisterPluginResponse>>, uint64_t matchId, std::string address, std::string password, std::string rconPassword, std::vector<SteamID> team1, std::vector<SteamID> team2) = 0;

        struct RegisterMatchResponse {
            std::string matchToken;
        };

        virtual void registerMatch(std::unique_ptr<Callback<RegisterMatchResponse>>, uint64_t matchId, std::string registrationToken) = 0;

        struct MatchResult {
            double homeTeamScore;
            double awayTeamScore;
            std::string pluginLogs;
        };

        virtual void submitMatch(std::unique_ptr<Callback<void>>, uint64_t matchId, std::string matchToken, MatchResult result) = 0;
    };
}
