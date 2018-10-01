#pragma once

#include "citadel/iclient.h"
#include "requests.h"

namespace citadel {
    class Client : public IClient {
        std::shared_ptr<Requests> requests;
        std::string endpoint;

    public:
        Client(std::shared_ptr<Requests> requests, std::string endpoint);
        ~Client();

        void findMatchForPlayers(
            IPlayer *invokerSteamID,
            std::vector<IPlayer *> playerSteamIDs,
            std::function<void (std::vector<Match> matches)> onResult,
            ErrorCallback onError) override;

        void registerPlugin(
            uint64_t matchId,
            IPlayer *starter,
            int port,
            std::string_view password,
            std::vector<IPlayer *> team1,
            std::vector<IPlayer *> team2,
            std::function<void (std::string registrationToken, std::string confirmationURL)> onResult,
            ErrorCallback onError) override;

        void registerMatch(
            uint64_t matchId,
            std::string registrationToken,
            std::function<void (std::string matchToken)> onResult,
            ErrorCallback onError) override;

        void submitMatch(
            uint64_t matchId,
            std::string matchToken,
            MatchResult result,
            std::function<void ()> onResult,
            ErrorCallback onError) override;
    };
}
