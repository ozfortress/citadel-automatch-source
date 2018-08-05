#include "citadel/client.h"

namespace citadel {
    Client::Client(std::shared_ptr<Requests> requests, std::string endpoint)
        : requests(requests), endpoint(endpoint) {}

    Client::~Client() {}

    void Client::findMatchForPlayers(
            SteamID invokerSteamID,
            std::vector<SteamID> playerSteamIDs,
            std::function<void (std::vector<Match> matches)> onResult,
            ErrorCallback onError) {
        // TODO
    }

    void Client::registerPlugin(
            uint64_t matchId,
            std::string address,
            std::string password,
            std::string rconPassword,
            std::vector<SteamID> team1,
            std::vector<SteamID> team2,
            std::function<void (std::string registrationToken, std::string confirmationURL)> onResult,
            ErrorCallback onError) {
        // TODO
    }

    void Client::registerMatch(
            uint64_t matchId,
            std::string registrationToken,
            std::function<void (std::string matchToken)> onResult,
            ErrorCallback onError) {
        // TODO
    }

    void Client::submitMatch(
            uint64_t matchId,
            std::string matchToken,
            MatchResult result,
            std::function<void ()> onResult,
            ErrorCallback onError) {
        // TODO
    }
}
