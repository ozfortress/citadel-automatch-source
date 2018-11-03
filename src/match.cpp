#include "match.h"
#include "utils.h"

#include <inttypes.h>
#include <iterator>
#include <sstream>
#include <time.h>

#include "utils.h"

Match::Match(IGame *game, std::shared_ptr<citadel::IClient> citadel, const citadel::Match& match)
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

void Match::onPlayerConfirm(std::string& confirmationURL, IPlayer *player) {
    player->openMOTD("AutoMatch Confirmation", confirmationURL);
}

void Match::start(IPlayer *starter) {
    printf("Registering plugin (%d, %d)\n", game == nullptr, citadel == nullptr);

    game->notifyAll(format("Starting match %s vs %s", matchInfo.homeTeam.name.c_str(), matchInfo.awayTeam.name.c_str()));

    if (matchInfo.rounds.size() == 0) {
        game->notifyAllError("Failed to start match. No available rounds.");
        game->resetMatch();
        return;
    }

    state = Initializing();

    citadel->registerPlugin(
        matchInfo.id,
        starter,
        game->serverPort(),
        game->serverRConPassword(),
        game->team1Players(),
        game->team2Players(),
        [=](std::string registrationToken, std::string confirmationURL) {
            state = ConfirmationPending(registrationToken, confirmationURL);

            game->notifyAll("Plugin registered. One player on each team please type !confirm and follow instructions to start the match.");
        },
        [=](int32_t code, std::string error) {
            game->notifyAllError(format("Failed to register plugin for match. Got error %d with message '%s'", code, error.c_str()));
            game->resetMatch();
        }
    );
}

bool Match::onCommand(IPlayer *player, std::string line) {
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
        [&](Running& value) {},
    }, state);

    return handled;
}

// TODO: Call this somehow
void Match::onServerConfirm() {
    auto confirmationPending = std::get_if<ConfirmationPending>(&state);
    if (confirmationPending == nullptr) {
        return;
    }

    citadel->registerMatch(
        matchInfo.id,
        confirmationPending->registrationToken,
        [=](std::string matchToken) {
            state = Running(matchToken);

            game->notifyAll(
                format("This server has been authorized to manage the match: %s vs %s\n",
                       matchInfo.homeTeam.name.c_str(), matchInfo.awayTeam.name.c_str()));
            game->notifyAll("Good luck and have fun!"); // TODO: Actual match management
        },
        [=](int32_t code, std::string error) {
            game->notifyAllError(format("Failed to register plugin for match. Got error %d with message '%s'", code, error.c_str()));
            game->resetMatch();
        }
    );
}

void Match::onServerConfirmationProgress() {
    auto confirmationPending = std::get_if<ConfirmationPending>(&state);
    if (confirmationPending == nullptr) {
        return;
    }

    // TODO: Request status update
    game->notifyAll("Registration has progressed\n");
}

void Match::onRoundWin(Team team) {
    auto running = std::get_if<Running>(&state);
    if (running == nullptr) return;

    sassert(team != Team::other, "Invalid round win team");

    if (team == homeTeam)
    {
        matchInfo.rounds[0].homeTeamScore++;
    }
    else
    {
        sassert(team == awayTeam, "Invalid round win team");
        matchInfo.rounds[0].awayTeamScore++;
    }

    game->notifyAll(format("Round was won: %" PRId32 " to %" PRId32 "\n", matchInfo.rounds[0].homeTeamScore, matchInfo.rounds[0].awayTeamScore));

    auto result = citadel::IClient::MatchResult(matchInfo.rounds, getLogs());

    citadel->updateMatch(
        matchInfo.id,
        running->matchToken,
        result,
        [=]() {
            game->notifyAll("Updated match results.");
        },
        [=](int32_t code, std::string error) {
            game->notifyAllError(format("Failed to send match update. Got error %d with message '%s'", code, error.c_str()));
        }
    );
}

void Match::onMapComplete() {
    auto running = std::get_if<Running>(&state);
    if (running == nullptr) return;

    game->notifyAll("Map Complete.");

    auto result = citadel::IClient::MatchResult(matchInfo.rounds, getLogs());

    citadel->submitMatch(
        matchInfo.id,
        running->matchToken,
        result,
        [=]() {
            game->notifyAll("Submitted match result.");
            game->resetMatch();
        },
        [=](int32_t code, std::string error) {
            game->notifyAllError(format("Failed to submit match. Got error %d with message '%s'", code, error.c_str()));
            // TODO: Retry
            game->resetMatch();
        }
    );
}
