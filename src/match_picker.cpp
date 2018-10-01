#include "match_picker.h"

#include "utils.h"

MatchPicker::MatchPicker(
        const IPlayer *starter,
        const std::vector<std::shared_ptr<citadel::IClient>>& clients,
        IGame *game)
        : starter(starter), clients(clients), game(game) {}

void MatchPicker::afterAllResults() {
    printf("Match picker completed with %zu matches\n", matches.size());

    if (matches.size() == 0) {
        starter->notifyError("There are no pending matches for the players in this server.");
        game->resetMatch();
        return;
    }

    // TODO: Improve message
    game->notifyAll("A match has been requested to start.\n\n");

    starter->notify("Please type `!N` where N is one of:");

    for (size_t i = 0; i < matches.size(); i++) {
        auto& match = matches[i];
        auto& homeTeam = match.details.homeTeam;
        auto& awayTeam = match.details.awayTeam;

        starter->notify(format("%zu. %s vs %s", i + 1, homeTeam.name.c_str(), awayTeam.name.c_str()));
    }
}

void MatchPicker::queryAll(std::vector<IPlayer *>& players) {
    for (auto& client : clients) {
        client->findMatchForPlayers(
            starter,
            players,
            [=](std::vector<citadel::Match> matches) {
                printf("Received %zu Matches\n", matches.size());

                for (auto& match : matches) {
                    this->matches.push_back(Match(client, match));
                }

                clientResults++;
                printf("%zu of %zu received\n", clientResults, clients.size());
                if (clientResults == clients.size()) afterAllResults();
            },
            [=](uint32_t code, std::string error) {
                clientResults++;
                if (clientResults == clients.size()) afterAllResults();

                // TODO: Print error
            });
    }
}

std::unique_ptr<Match> MatchPicker::onCommand(IPlayer *player, std::string command) {
    if (player != starter) return nullptr;
    if (clientResults != clients.size()) return nullptr;

    int32_t matchIndex = atoi(command.c_str());

    if (matchIndex <= 0 || matchIndex > matches.size()) {
        starter->notify("Please select a valid match.");
        // TODO: Send an error

        return nullptr;
    }

    auto& match = matches[matchIndex - 1];

    printf("Starting match: %s vs %s\n", match.details.homeTeam.name.c_str(), match.details.awayTeam.name.c_str());

    return std::make_unique<::Match>(std::move(game), std::move(match.client), match.details);
}
