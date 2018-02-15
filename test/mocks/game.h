#pragma once

#include "igame.h"

namespace mocks {
    class Game : public IGame {
    public:
        std::vector<SteamID> team1;
        std::vector<SteamID> team2;

        Game();
        ~Game();

        std::string serverAddress() override;
        std::string serverPassword() override;
        std::string serverRConPassword() override;
        std::vector<SteamID> team1Players() override;
        std::vector<SteamID> team2Players() override;

        void notifyError(std::string message, SteamID target) override;
        void resetMatch() override;
    };
}
