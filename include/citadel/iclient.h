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

    struct Round {
        uint64_t id;
        std::string map;
        int32_t homeTeamScore = 0;
        int32_t awayTeamScore = 0;

        Round(uint64_t i, std::string && m, int32_t h, int32_t a) : id(i), map(m), homeTeamScore(h), awayTeamScore(a) {}
    };

    struct Match {
        uint64_t id;
        Roster homeTeam;
        Roster awayTeam;
        std::vector<Round> rounds;

        Match(uint64_t i, Roster h, Roster a, std::vector<Round> && r) : id(i), homeTeam(h), awayTeam(a), rounds(r) {}
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
            std::vector<Round> rounds;
            std::string pluginLogs;

            MatchResult(std::vector<Round> r, std::string l)
                : rounds(r)
                , pluginLogs(l) {}
        };

        virtual void updateMatch(
            uint64_t matchId,
            std::string matchToken,
            MatchResult result,
            std::function<void ()> onResult,
            ErrorCallback onError) = 0;

        virtual void submitMatch(
            uint64_t matchId,
            std::string matchToken,
            MatchResult result,
            std::function<void ()> onResult,
            ErrorCallback onError) = 0;
    };
}
