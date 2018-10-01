#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <functional>

#include "igame.h"
#include "steam_id.h"

namespace citadel {
    struct Roster {
        uint64_t id;
        std::string name;

        Roster(uint64_t i, std::string n) : id(i), name(n) {}
    };

    struct Match {
        uint64_t id;
        Roster homeTeam;
        Roster awayTeam;

        Match(uint64_t i, Roster h, Roster a) : id(i), homeTeam(h), awayTeam(a) {}
    };

    class IClient {
    public:
        using ErrorCallback = std::function<void (int32_t, std::string)>;

        virtual void findMatchForPlayers(
            IPlayer *invokerSteamID,
            std::vector<IPlayer *> playerSteamIDs,
            std::function<void (std::vector<Match> matches)> onResult,
            ErrorCallback onError) = 0;

        virtual void registerPlugin(
            uint64_t matchId,
            IPlayer *starter,
            int port,
            std::string_view password,
            std::vector<IPlayer *> team1,
            std::vector<IPlayer *> team2,
            std::function<void (std::string registrationToken, std::string confirmationURL)> onResult,
            ErrorCallback onError) = 0;

        virtual void registerMatch(
            uint64_t matchId,
            std::string registrationToken,
            std::function<void (std::string matchToken)> onResult,
            ErrorCallback onError) = 0;

        struct MatchResult {
            double homeTeamScore;
            double awayTeamScore;
            std::string pluginLogs;

            MatchResult(double h, double a, std::string l)
                : homeTeamScore(h)
                , awayTeamScore(a)
                , pluginLogs(l) {}
        };

        virtual void submitMatch(
            uint64_t matchId,
            std::string matchToken,
            MatchResult result,
            std::function<void ()> onResult,
            ErrorCallback onError) = 0;
    };
}
