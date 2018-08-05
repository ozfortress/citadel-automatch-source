#include "match_picker.h"

void MatchPicker::afterAllResults() {

}

void MatchPicker::queryAll(SteamID starter, std::vector<SteamID> &players) {
    this->starter = starter;

    for (auto &client : clients) {
        client->findMatchForPlayers(
            starter,
            players,
            [=](std::vector<citadel::Match> matches) {
                clientResults++;
                if (clientResults == clients.size()) afterAllResults();

                for (auto &match : matches) {
                    this->matches.push_back(Match(client, match));
                }
            },
            [=](uint32_t code, std::string error) {
                clientResults++;
                if (clientResults == clients.size()) afterAllResults();

                // TODO: Print error
            });
    }
}

std::unique_ptr<Match> MatchPicker::onCommand(std::string command, SteamID id, Team team) {
    if (id != starter) return nullptr;

    int32_t matchIndex = atoi(command.c_str());

    if (matchIndex <= 0 || matchIndex > matches.size()) {
        // TODO: Send an error

        return nullptr;
    }

    auto &match = matches[matchIndex + 1];
    return std::make_unique<::Match>(nullptr, match.client, match.details);
}
