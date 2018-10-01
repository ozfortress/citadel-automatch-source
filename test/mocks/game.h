#pragma once

#include <memory>

#include "igame.h"

namespace mocks {
    class Player : public IPlayer {
    public:
        SteamID steam_id;
        Team team;
        std::string name;

        Player(SteamID steam_id = SteamID::null, Team team = Team::other, std::string name = "Player");
        ~Player();

        const std::string_view getName() const override;
        void setName(const std::string_view) override;
        SteamID getSteamID() const override;
        Team getTeam() const override;

        void notify(const std::string_view) override;
        void notifyError(const std::string_view) override;

        void openMOTD(const std::string_view title, const std::string_view url) override;
    };

    class Game : public IGame {
    public:
        std::vector<IPlayer *> players;

        Game(std::vector<IPlayer *>);
        Game(std::vector<std::unique_ptr<IPlayer>>&);
        ~Game();

        int serverPort() const override;
        const std::string_view serverPassword() const override;
        const std::string_view serverRConPassword() const override;
        const std::string_view serverName() const override;

        std::vector<IPlayer *> team1Players() const override;
        std::vector<IPlayer *> team2Players() const override;
        std::vector<IPlayer *> nonTeamPlayers() const override;

        void resetMatch() override;

        void notifyAll(std::string_view) override;
        void notifyAllError(std::string_view) override;
    };
}
