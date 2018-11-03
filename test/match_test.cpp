#include "catch.hpp"

#include <memory>

#include "match.h"

#include "mocks.h"

citadel::Match match_details(34, citadel::Roster(1, "foo"), citadel::Roster(2, "bar"), {});

TEST_CASE("Match") {
    SECTION("::start") {
        SECTION("works normally") {
            struct CitadelClient : public mocks::CitadelClient {
                int calls = 0;

                void registerPlugin(
                        uint64_t matchId,
                        IPlayer *starter,
                        int port,
                        std::string_view password,
                        std::vector<IPlayer *> team1,
                        std::vector<IPlayer *> team2,
                        std::function<void (std::string registrationToken, std::string confirmationURL)> onResult,
                        ErrorCallback onError) override {
                    REQUIRE(matchId == 34);
                    REQUIRE(starter->getSteamID() == SteamID(3));
                    REQUIRE(port == 1234);
                    REQUIRE(password == "RConPassword");
                    REQUIRE(team1.size() == 1);
                    REQUIRE(team1[0]->getSteamID() == SteamID(3));
                    REQUIRE(team2.size() == 1);
                    REQUIRE(team2[0]->getSteamID() == SteamID(4));

                    calls++;

                    const auto registrationToken = "registration";
                    const auto confirmationURL = "https://warzone.ozfortress.com/plugins/source/automatch/plugin/confirm?token=foo";
                    onResult(registrationToken, confirmationURL);
                }
            };

            auto citadel = std::make_shared<CitadelClient>();

            std::vector<std::unique_ptr<IPlayer>> players;
            players.push_back(std::make_unique<mocks::Player>(SteamID(3), Team::team1));
            players.push_back(std::make_unique<mocks::Player>(SteamID(4), Team::team2));

            auto game = std::make_unique<mocks::Game>(players);
            auto match = std::make_unique<Match>(game.get(), citadel, match_details);

            match->start(players[0].get());

            REQUIRE(citadel->calls == 1);
        }

        SECTION("resets match on request error") {
            struct Game : public mocks::Game {
                using mocks::Game::Game;

                int notify_error_calls = 0;
                int reset_match_calls = 0;

                void notifyAllError(std::string_view message) override {
                    notify_error_calls++;
                    // TODO: Require something about the message
                }

                void resetMatch() override {
                    reset_match_calls++;
                }
            };

            struct CitadelClient : public mocks::CitadelClient {
                int calls = 0;

                void registerPlugin(
                        uint64_t matchId,
                        IPlayer *starter,
                        int port,
                        std::string_view password,
                        std::vector<IPlayer *> team1,
                        std::vector<IPlayer *> team2,
                        std::function<void (std::string registrationToken, std::string confirmationURL)> onResult,
                        ErrorCallback onError) override {
                    onError(500, "Internal Server Error");
                    calls++;
                }
            };

            auto citadel = std::make_shared<CitadelClient>();

            std::vector<std::unique_ptr<IPlayer>> players;
            players.push_back(std::make_unique<mocks::Player>());

            auto game = std::make_unique<Game>(players);
            auto match = std::make_unique<Match>(game.get(), citadel, match_details);

            match->start(players[0].get());

            REQUIRE(citadel->calls == 1);
            REQUIRE(game->notify_error_calls == 1);
            REQUIRE(game->reset_match_calls == 1);
        }
    }

    SECTION("::onCommand") {
        SECTION("in ConfirmationPending state") {
            SECTION("!confirm") {
                struct Player : public mocks::Player {
                    std::string motd_title = "";
                    std::string motd_url = "";

                    void openMOTD(std::string_view title, std::string_view url) override {
                        motd_title = title;
                        motd_url = url;
                    }
                };

                auto citadel = std::make_shared<mocks::CitadelClient>();
                auto player1 = std::make_unique<Player>();
                auto player2 = std::make_unique<Player>();
                std::vector<IPlayer *> players = { player1.get(), player2.get() };
                auto game = std::make_unique<mocks::Game>(players);
                auto match = std::make_unique<Match>(game.get(), citadel, match_details);

                std::string url("http://example.com/confirm");
                match->state = Match::ConfirmationPending("123", url);

                REQUIRE(match->onCommand(player1.get(), "confirm") == true);

                // REQUIRE(player1->motd_title == "something");
                REQUIRE(player1->motd_url == url);
                player1->motd_url = "";

                REQUIRE(match->onCommand(player2.get(), "confirm ") == true);

                REQUIRE(player2->motd_url == url);
                player2->motd_url = "";

                REQUIRE(match->onCommand(player1.get(), "conifrm") == true);

                REQUIRE(player1->motd_url == url);
                player1->motd_url = "";

                REQUIRE(match->onCommand(player2.get(), "cinfurn") == false);

                REQUIRE(player2->motd_url == "");
            }
        }
    }

    SECTION("::onServerConfirm") {
        // TODO
    }

    SECTION("::onMatchComplete") {

    }
}
