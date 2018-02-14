#include "mocks/citadel_client.h"

mocks::CitadelClient::CitadelClient() {}

mocks::CitadelClient::~CitadelClient() {}

void mocks::CitadelClient::findMatchForPlayers(Callback<std::vector<citadel::Match>> callback, SteamID invokerSteamID, std::vector<SteamID> playerSteamIDs) {

}

void mocks::CitadelClient::registerPlugin(Callback<RegisterPluginResponse> callback, uint64_t matchId) {

}

void mocks::CitadelClient::registerMatch(Callback<RegisterMatchResponse> callback, uint64_t matchId, std::string registrationToken) {

}

void mocks::CitadelClient::submitMatch(Callback<> callback, uint64_t matchId, std::string matchToken, MatchResult result) {

}
