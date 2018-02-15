#include "catch.hpp"

#include <memory>

#include "match.h"

#include "mocks.h"

TEST_CASE("Match::start") {
    SECTION("works normally") {
        struct CitadelClient : public mocks::CitadelClient {
            int calls = 0;

            void registerPlugin(std::unique_ptr<Callback<RegisterPluginResponse>> callback, uint64_t matchId, std::string address, std::string password, std::string rconPassword, std::vector<SteamID> team1, std::vector<SteamID> team2) override {
                REQUIRE(matchId == 34);
                REQUIRE(address == "127.0.0.1");
                REQUIRE(password == "Password");
                REQUIRE(rconPassword == "RConPassword");
                calls++;

                std::unique_ptr<RegisterPluginResponse> result(new RegisterPluginResponse());
                result->registrationToken = "registration";
                result->confirmationURL = "https://warzone.ozfortress.com/plugins/source/automatch/plugin/confirm?token=foo";
                callback->onResult(std::move(result));
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

            void registerPlugin(std::unique_ptr<Callback<RegisterPluginResponse>> callback, uint64_t matchId, std::string address, std::string password, std::string rconPassword, std::vector<SteamID> team1, std::vector<SteamID> team2) override {
                callback->onError(500, "Internal Server Error");
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
