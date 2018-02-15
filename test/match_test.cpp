#include "catch.hpp"

#include <memory>

#include "match.h"

#include "mocks.h"

TEST_CASE("Match::start") {
    SECTION("works normally") {
        class CitadelClient : public mocks::CitadelClient {
        public:
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
}

// TEST_CASE( "Factorials are computed", "[factorial]" ) {
//     std::shared_ptr<citadel::IClient> citadel(new mocks::CitadelClient());
//     std::shared_ptr<IGame> game(new mocks::Game());
//     Match match(game, citadel, 1);

//     match.log("Foo");
//     match.log("Bar");

//     REQUIRE(match.getLogs() == "Foo\nBar\n");
// }