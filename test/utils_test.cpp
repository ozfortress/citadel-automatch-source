#include "catch.hpp"

#include "utils.h"

TEST_CASE("format()") {
    SECTION("basic string formatting") {
        REQUIRE(format("foo %s foo %s", "bar", "baz") == "foo bar foo baz");

        std::string foo = "test";
        REQUIRE(format("foo %s", foo.c_str()) == "foo test");
    }

    SECTION("complex formatting") {
        auto result = format("%s away", format("%d%s", 12, "km").c_str());
        REQUIRE(result == "12km away");
    }
}
