#include "match.h"
#include "utils.h"

#include <sstream>
#include <iterator>

#include "utils.h"

template<class ... Types>
struct CitadelCallback : citadel::IClient::Callback<Types ...> {
    Match& match;

    explicit CitadelCallback(Match& match) : match(match) {}
    ~CitadelCallback() {}
};

Match::Match(std::shared_ptr<IGame> game, std::shared_ptr<citadel::IClient> citadel, uint64_t matchId)
        : citadel(std::move(citadel)),
          game(std::move(game)),
          matchId(matchId) {}

Match::~Match() {}

void Match::log(std::string content) {
    logs.push_back(content);
}

std::string Match::getLogs() {
    std::ostringstream joined;
    std::copy(logs.begin(), logs.end(), std::ostream_iterator<std::string>(joined, "\n"));
    return joined.str();
}

void Match::start() {
    state = State::initializing;

    struct Callback : public CitadelCallback<citadel::IClient::RegisterPluginResponse> {
        explicit Callback(Match& match) : CitadelCallback(match) {}

        void onResult(std::unique_ptr<citadel::IClient::RegisterPluginResponse> response) override {
            match.state = ConfirmationPending();

            match.game->notifyAll("Plugin registered. Can one player of each team please type '!confirm' to confirm the rosters.");
        }

        void onError(int32_t code, std::string error) override {
            match.game->notifyError(format("Failed to register plugin for match. Got error %d with message '%s'", code, error.c_str()));
            match.game->resetMatch();
        }
    };
    std::unique_ptr<Callback> callback(new Callback(*this));

    citadel->registerPlugin(
        std::move(callback),
        matchId,
        game->serverAddress(),
        game->serverPassword(),
        game->serverRConPassword(),
        game->team1Players(),
        game->team2Players()
    );
}

bool Match::onCommand(std::string line, SteamID player, Team team) {
    trim(line);

    auto handled = false;

    std::visit(overloaded {
        [&](Initializing& value) {

        },
        [&](ConfirmationPending& value) {
            if (levenshtein_distance(line, "confirm") <= 2) {


                handled = true;
            }
        },
        [&](Running& value) {

        },
    }, state);

    return handled;
}
