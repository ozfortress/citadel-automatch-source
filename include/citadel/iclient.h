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
        template<class ... Types>
        class Callback {
            void onResult(Types ... args);
            void onError(int32_t code, std::string error);
        };

        virtual void findMatchForPlayers(Callback<std::vector<Match>>, SteamID invokerSteamID, std::vector<SteamID> playerSteamIDs) = 0;

        struct RegisterPluginResponse {
            std::string registrationToken;
            std::string confirmationURL;
            std::string matchRegistrationURL;
        };

        virtual void registerPlugin(Callback<RegisterPluginResponse>, uint64_t matchId) = 0;

        struct RegisterMatchResponse {
            enum class State {
                pending,
                canceled,
                error
            };

            State state;
            std::string matchToken;
        };

        virtual void registerMatch(Callback<RegisterMatchResponse>, uint64_t matchId, std::string registrationToken) = 0;


        struct MatchResult {
            double homeTeamScore;
            double awayTeamScore;
            std::string pluginLogs;
        };

        virtual void submitMatch(Callback<>, uint64_t matchId, std::string matchToken, MatchResult result) = 0;
    };
}
