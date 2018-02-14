#include "main.h"

#include <memory>

#include "match.h"

#include "mocks.h"

TEST_CASE( "Factorials are computed", "[factorial]" ) {
    std::shared_ptr<citadel::IClient> citadel(new mocks::CitadelClient());
    std::shared_ptr<IGame> game(new mocks::Game());
    Match match(game, citadel);

    match.log("Foo");
    match.log("Bar");

    REQUIRE(match.getLogs() == "Foo\nBar\n");
}
