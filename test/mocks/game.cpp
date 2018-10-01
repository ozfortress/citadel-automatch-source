#include "mocks/game.h"

namespace mocks {
    Player::Player(SteamID steam_id, Team team, std::string name)
        : steam_id(steam_id), team(team), name(name) {

    }

    Player::~Player() {}

    const std::string_view Player::getName() const {
        return name;
    }

    void Player::setName(const std::string_view name) {
        this->name = name;
    }

    SteamID Player::getSteamID() const {
        return steam_id;
    }

    Team Player::getTeam() const {
        return team;
    }

    void Player::notify(const std::string_view) {}

    void Player::notifyError(const std::string_view) {}

    void Player::openMOTD(const std::string_view title, const std::string_view url) {}

    Game::Game(std::vector<IPlayer *> players) : players(std::move(players)) {}

    Game::Game(std::vector<std::unique_ptr<IPlayer>>& players) {
        this->players.reserve(players.size());
        for (auto& player : players) {
            this->players.push_back(player.get());
        }
    }

    Game::~Game() {}

    int Game::serverPort() const {
        return 1234;
    }

    const std::string_view Game::serverPassword() const {
        return "Password";
    }

    const std::string_view Game::serverRConPassword() const {
        return "RConPassword";
    }

    const std::string_view Game::serverName() const {
        return "Test Server";
    }

    std::vector<IPlayer *> Game::team1Players() {
        std::vector<IPlayer *> players;
        for (auto player : this->players) {
            if (player->getTeam() == Team::team1) {
                players.push_back(player);
            }
        }
        return players;
    }

    std::vector<IPlayer *> Game::team2Players() {
        std::vector<IPlayer *> players;
        for (auto player : this->players) {
            if (player->getTeam() == Team::team2) {
                players.push_back(player);
            }
        }
        return players;
    }

    std::vector<IPlayer *> Game::nonTeamPlayers() {
        std::vector<IPlayer *> players;
        for (auto player : this->players) {
            if (player->getTeam() == Team::other) {
                players.push_back(player);
            }
        }
        return players;
    }

    void Game::resetMatch() {}

    void Game::notifyAll(std::string_view) {}

    void Game::notifyAllError(std::string_view) {}
}
