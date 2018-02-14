#include "match.h"

#include <sstream>
#include <iterator>

Match::Match(std::shared_ptr<IGame> game, std::shared_ptr<citadel::IClient> citadel) : citadel(std::move(citadel)), game(std::move(game)) {}

Match::~Match() {}

void Match::log(std::string content) {
    logs.push_back(content);
}

std::string Match::getLogs() {
    std::ostringstream joined;
    std::copy(logs.begin(), logs.end(), std::ostream_iterator<std::string>(joined, "\n"));
    return joined.str();
}

void Match::start(SteamID initiator) {

}
