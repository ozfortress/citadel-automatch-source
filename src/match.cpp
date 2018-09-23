#include "match.h"
#include "utils.h"

#include <sstream>
#include <iterator>
#include <time.h>

#include "utils.h"

Match::Match(std::shared_ptr<IGame> game, std::shared_ptr<citadel::IClient> citadel, const citadel::Match& match)
        : game(std::move(game))
        , citadel(std::move(citadel))
        , matchInfo(match) {}

Match::~Match() {}

void Match::log(std::string content) {
    // Log the timestamp as well
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "%Y-%VT%H:%M:%S%z", timeinfo);

    logs.push_back(format("%s | %s", buffer, content));
}

std::string Match::getLogs() {
    std::ostringstream joined;
    std::copy(logs.begin(), logs.end(), std::ostream_iterator<std::string>(joined, "\n"));
    return joined.str();
}

void Match::onPlayerConfirm(std::string& confirmationURL, SteamID player) {
    game->openMOTD(player, "AutoMatch Confirmation", confirmationURL);
}

void Match::onMatchComplete(uint32_t homeTeamScore, uint32_t awayTeamScore) {
    auto running = std::get_if<Running>(&state);
    assert(running != nullptr, "Invalid state");

    auto result = citadel::IClient::MatchResult(homeTeamScore, awayTeamScore, getLogs());

    citadel->submitMatch(
        matchInfo.id,
        running->matchToken,
        result,
        [=]() {
            // TODO: Kill match
            game->notifyError(format("Submitted match result."));
        },
        [=](int32_t code, std::string error) {
            // TODO: Retry
            game->notifyError(format("Failed to submit match. Get error %d with message '%s'", code, error.c_str()));
        }
    );
}

void Match::start(SteamID starter) {
    printf("Registering plugin (%d, %d)\n", game == nullptr, citadel == nullptr);

    game->notifyAll(format("Starting match %s vs %s", matchInfo.homeTeam.name.c_str(), matchInfo.awayTeam.name.c_str()));

    state = Initializing();

    citadel->registerPlugin(
        matchInfo.id,
        starter,
        game->serverAddress(),
        game->serverPassword(),
        game->serverRConPassword(),
        game->team1Players(),
        game->team2Players(),
        [=](std::string registrationToken, std::string confirmationURL) {
            state = ConfirmationPending(registrationToken, confirmationURL);

            game->notifyAll("Plugin registered. One player on each team please type !confirm and follow instructions to start the match.");
        },
        [=](int32_t code, std::string error) {
            game->notifyError(format("Failed to register plugin for match. Got error %d with message '%s'", code, error.c_str()));
            game->resetMatch();
        }
    );
}

bool Match::onCommand(SteamID player, std::string line) {
    trim(line);

    auto handled = false;

    std::visit(overloaded {
        // No commands in the initialization state
        [&](Initializing& value) {},
        [&](ConfirmationPending& value) {
            if (levenshtein_distance(line, "confirm") <= 2) {
                onPlayerConfirm(value.confirmationURL, player);

                handled = true;
            }

            if (levenshtein_distance(line, "cancel") <= 2) {
                game->resetMatch();
            }
        },
        [&](Running& value) {
            if (line == "complete") {
                onMatchComplete(1, 0);

                handled = true;
            }
        },
    }, state);

    return handled;
}

// TODO: Call this somehow
void Match::onServerConfirm() {
    auto confirmationPending = std::get_if<ConfirmationPending>(&state);
    assert(confirmationPending != nullptr, "Invalid state");

    citadel->registerMatch(
        matchInfo.id,
        confirmationPending->registrationToken,
        [=](std::string matchToken) {
            state = Running(matchToken);
        },
        [=](int32_t code, std::string error) {
            // TODO
        }
    );
}
