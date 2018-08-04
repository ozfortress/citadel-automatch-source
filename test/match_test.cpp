#include "catch.hpp"

#include <memory>

#include "match.h"

#include "mocks.h"

TEST_CASE("Match") {
    SECTION("::start") {
        SECTION("works normally") {
            struct CitadelClient : public mocks::CitadelClient {
                int calls = 0;

                void registerPlugin(uint64_t matchId,
                        std::string address,
                        std::string password,
                        std::string rconPassword,
                        std::vector<SteamID> team1,
                        std::vector<SteamID> team2,
                        std::function<void (std::string registrationToken, std::string confirmationURL)> onResult,
                        ErrorCallback onError) override {
                    REQUIRE(matchId == 34);
                    REQUIRE(address == "127.0.0.1");
                    REQUIRE(password == "Password");
                    REQUIRE(rconPassword == "RConPassword");
                    calls++;

                    const auto registrationToken = "registration";
                    const auto confirmationURL = "https://warzone.ozfortress.com/plugins/source/automatch/plugin/confirm?token=foo";
                    onResult(registrationToken, confirmationURL);
                }
            };

            std::shared_ptr<CitadelClient> citadel(new CitadelClient());
            std::shared_ptr<IGame> game(new mocks::Game());
            std::unique_ptr<Match> match(new Match(game, citadel, 34));
            match->start();

            REQUIRE(citadel->calls == 1);
        }

        SECTION("resets match on request error") {
            struct Game : public mocks::Game {
                int notifyErrorCalls = 0;
                int resetMatchCalls = 0;

                void notifyError(std::string message, SteamID target) override {
                    notifyErrorCalls++;
                    REQUIRE(target == SteamID(0)); // It should notify everyone
                    // TODO: Require something about the message
                }

                void resetMatch() override {
                    resetMatchCalls++;
                }
            };

            struct CitadelClient : public mocks::CitadelClient {
                int calls = 0;

                void registerPlugin(uint64_t matchId,
                        std::string address,
                        std::string password,
                        std::string rconPassword,
                        std::vector<SteamID> team1,
                        std::vector<SteamID> team2,
                        std::function<void (std::string registrationToken, std::string confirmationURL)> onResult,
                        ErrorCallback onError) override {
                    onError(500, "Internal Server Error");
                    calls++;
                }
            };

            std::shared_ptr<CitadelClient> citadel(new CitadelClient());
            std::shared_ptr<Game> game(new Game());
            std::unique_ptr<Match> match(new Match(game, citadel, 34));
            match->start();

            REQUIRE(citadel->calls == 1);
            REQUIRE(game->notifyErrorCalls == 1);
            REQUIRE(game->resetMatchCalls == 1);
        }
    }

    SECTION("::onCommand") {
        SECTION("in ConfirmationPending state") {
            SECTION("!confirm") {
                struct Game : public mocks::Game {
                    SteamID playerMOTD = SteamID(0);
                    std::string urlMOTD = "";

                    void openMOTD(SteamID player, std::string url) {
                        playerMOTD = player;
                        urlMOTD = url;
                    }
                };

                std::shared_ptr<citadel::IClient> citadel(nullptr);
                std::shared_ptr<Game> game(new Game());
                std::unique_ptr<Match> match(new Match(game, citadel, 34));

                std::string url("http://example.com/confirm");
                match->state = Match::ConfirmationPending("123", url);

                SteamID player1(123);
                REQUIRE(match->onCommand("confirm", player1, Team::team1) == true);

                REQUIRE(game->playerMOTD == player1);
                REQUIRE(game->urlMOTD == url);

                SteamID player2(456);
                REQUIRE(match->onCommand("confirm ", player2, Team::team1) == true);

                REQUIRE(game->playerMOTD == player2);
                REQUIRE(game->urlMOTD == url);

                REQUIRE(match->onCommand("conifrm", player1, Team::team2) == true);

                REQUIRE(game->playerMOTD == player1);
                REQUIRE(game->urlMOTD == url);

                REQUIRE(match->onCommand("cinfurn", player2, Team::team2) == false);

                REQUIRE(game->playerMOTD == player1);
                REQUIRE(game->urlMOTD == url);
            }
        }
    }

    SECTION("::onServerConfirm") {
        // TODO
    }

    SECTION("::onMatchComplete") {

    }
}
