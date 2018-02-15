#include "mocks/game.h"

mocks::Game::Game() {}

mocks::Game::~Game() {}

std::string mocks::Game::serverAddress() {
    return "127.0.0.1";
}

std::string mocks::Game::serverPassword() {
    return "Password";
}

std::string mocks::Game::serverRConPassword() {
    return "RConPassword";
}

std::vector<SteamID> mocks::Game::team1Players() {
    return team1;
}

std::vector<SteamID> mocks::Game::team2Players() {
    return team2;
}

void mocks::Game::notifyError(std::string message, SteamID target) {}

void mocks::Game::resetMatch() {}

