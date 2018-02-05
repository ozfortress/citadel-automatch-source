#include "match.h"

#include <sstream>
#include <iterator>

Match::Match() {}

Match::~Match() {}

void Match::log(std::string content) {
    logs.push_back(content);
}

std::string Match::getLogs() {
    std::ostringstream joined;
    std::copy(logs.begin(), logs.end(), std::ostream_iterator<std::string>(joined, "\n"));
    return joined.str();
}
