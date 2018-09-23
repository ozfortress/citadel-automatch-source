#include "citadel/client.h"

#include "utils.h"

#include <string>

namespace citadel {
    Client::Client(std::shared_ptr<Requests> requests, std::string endpoint)
        : requests(requests), endpoint(endpoint) {}

    Client::~Client() {}

    void Client::findMatchForPlayers(
            SteamID invokerSteamID,
            std::vector<SteamID> playerSteamIDs,
            std::function<void (std::vector<Match> matches)> onResult,
            ErrorCallback onError) {
        std::string url = format(
            "%s/api/v1/auto_match/find?invoker=%"PRIu64"&players[]=%"PRIu64"", endpoint.c_str(), invokerSteamID.value, invokerSteamID.value);

        Requests::Request req(Requests::Method::GET, url);
        requests->request(req,
            [=](const Requests::Response& res) {
                if (res.json.HasMember("matches")) {
                    auto& matches = res.json["matches"];
                    assert(matches.IsArray(), "Wrong json type");

                    std::vector<Match> result;
                    for (size_t i = 0; i < matches.Size(); i++) {
                        auto& match = matches[i];
                        assert(match.IsObject(), "Wrong json type");

                        auto& homeTeam = match["home_team"];
                        auto& awayTeam = match["away_team"];

                        result.push_back(
                            Match(
                                match["id"].GetUint64(),
                                Roster(
                                    homeTeam["id"].GetUint64(),
                                    homeTeam["name"].GetString()),
                                Roster(
                                    awayTeam["id"].GetUint64(),
                                    awayTeam["name"].GetString())));
                    }
                    onResult(result);
                } else if (res.json.HasMember("errors")) {
                    // TODO: Better error handling
                    onError(res.code, res.json["errors"][0].GetString());
                } else {
                    onError(0, "Internal Error");
                }
            },
            [=](const std::string& msg) {
                onError(0, msg);
            });
    }

    void Client::registerPlugin(
            uint64_t matchId,
            SteamID starter,
            std::string address,
            std::string password,
            std::string rconPassword,
            std::vector<SteamID> team1,
            std::vector<SteamID> team2,
            std::function<void (std::string registrationToken, std::string confirmationURL)> onResult,
            ErrorCallback onError) {
        std::string url = endpoint + "/api/v1/auto_match/register";

        std::vector<std::pair<std::string, std::string>> params;
        params.push_back(std::pair("id", std::to_string(matchId)));
        params.push_back(std::pair("invoker", std::to_string(starter.value)));
        Requests::Request req(Requests::Method::POST, url, params);

        requests->request(req,
            [=](const Requests::Response& res) {
                if (!res.json.HasMember("registration")) {
                    if (res.json.HasMember("errors")) {
                        onError(res.code, res.json["errors"][0].GetString());
                    }
                    onError(0, "Internal Error");
                    return;
                }
                auto& registration = res.json["registration"];

                std::string token = registration["token"].GetString();
                std::string verifyUrl = registration["verify_url"].GetString();
                onResult(token, verifyUrl);
            },
            [=](const std::string& msg) {
                onError(0, msg);
            });
    }

    void Client::registerMatch(
            uint64_t matchId,
            std::string registrationToken,
            std::function<void (std::string matchToken)> onResult,
            ErrorCallback onError) {
        // TODO
    }

    void Client::submitMatch(
            uint64_t matchId,
            std::string matchToken,
            MatchResult result,
            std::function<void ()> onResult,
            ErrorCallback onError) {
        // TODO
    }
}
