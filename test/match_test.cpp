#include "main.h"

#include <memory>

#include "match.h"

TEST_CASE( "Factorials are computed", "[factorial]" ) {
    Match match;

    match.log("Foo");
    match.log("Bar");

    REQUIRE(match.getLogs() == "Foo\nBar\n");
}
