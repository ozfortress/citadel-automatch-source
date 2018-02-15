#include "mocks/citadel_client.h"

mocks::CitadelClient::CitadelClient() {}

mocks::CitadelClient::~CitadelClient() {}

void mocks::CitadelClient::findMatchForPlayers(std::unique_ptr<Callback<MatchForPlayersResponse>> callback, SteamID invokerSteamID, std::vector<SteamID> playerSteamIDs) {

}

void mocks::CitadelClient::registerPlugin(std::unique_ptr<Callback<RegisterPluginResponse>> callback, uint64_t matchId, std::string address, std::string password, std::string rconPassword, std::vector<SteamID> team1, std::vector<SteamID> team2) {

}

void mocks::CitadelClient::registerMatch(std::unique_ptr<Callback<RegisterMatchResponse>> callback, uint64_t matchId, std::string registrationToken) {

}

void mocks::CitadelClient::submitMatch(std::unique_ptr<Callback<void>> callback, uint64_t matchId, std::string matchToken, MatchResult result) {

}
