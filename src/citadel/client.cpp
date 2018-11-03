#include "citadel/client.h"

#include "utils.h"

#include <string>

namespace citadel {
    Client::Client(std::shared_ptr<Requests> requests, std::string endpoint)
        : requests(requests), endpoint(endpoint) {}

    Client::~Client() {}

    void Client::findMatchForPlayers(
            IPlayer *invoker,
            std::vector<IPlayer *> players,
            std::function<void (std::vector<Match> matches)> onResult,
            ErrorCallback onError) {
        // TODO: Player steamIDs
        std::string url = format(
            "%s/api/v1/auto_match/find?invoker=%" PRIu64 "&players[]=%" PRIu64 "", endpoint.c_str(), invoker->getSteamID().value, invoker->getSteamID().value);

        Requests::Request req(Requests::Method::GET, url);
        requests->request(req,
            [=](const Requests::Response& res) {
                if (res.json.HasMember("matches")) {
                    auto& matches = res.json["matches"];
                    sassert(matches.IsArray(), "Wrong json type");

                    std::vector<Match> result;
                    for (size_t i = 0; i < matches.Size(); i++) {
                        auto& match = matches[i];
                        sassert(match.IsObject(), "Wrong json type");

                        auto& homeTeam = match["home_team"];
                        auto& awayTeam = match["away_team"];

                        auto& roundsJson = match["rounds"];
                        sassert(roundsJson.IsArray(), "Wrong json type");
                        std::vector<Round> rounds;
                        for (size_t j = 0; j < roundsJson.Size(); j++) {
                            auto& round = roundsJson[j];
                            rounds.push_back(
                                Round(round["id"].GetUint64(), "", 0, 0));
                        }

                        result.push_back(
                            Match(
                                match["id"].GetUint64(),
                                Roster(
                                    homeTeam["id"].GetUint64(),
                                    homeTeam["name"].GetString()),
                                Roster(
                                    awayTeam["id"].GetUint64(),
                                    awayTeam["name"].GetString()),
                                std::move(rounds)));
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
            IPlayer *starter,
            int port,
            std::string_view password,
            std::vector<IPlayer *> team1,
            std::vector<IPlayer *> team2,
            std::function<void (std::string registrationToken, std::string confirmationURL)> onResult,
            ErrorCallback onError) {
        std::string url = endpoint + "/api/v1/auto_match/register";

        std::vector<std::pair<std::string, std::string>> params;
        params.push_back(std::pair("id", std::to_string(matchId)));
        params.push_back(std::pair("invoker", std::to_string(starter->getSteamID().value)));
        params.push_back(std::pair("port", std::to_string(port)));
        params.push_back(std::pair("password", std::string(password)));
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
        std::string url = endpoint + "/api/v1/auto_match/authorize";

        std::vector<std::pair<std::string, std::string>> params;
        params.push_back(std::pair("id", std::to_string(matchId)));
        params.push_back(std::pair("token", registrationToken));
        Requests::Request req(Requests::Method::POST, url, params);

        requests->request(req,
            [=](const Requests::Response& res) {
                if (!res.json.HasMember("authorization")) {
                    if (res.json.HasMember("errors")) {
                        onError(res.code, res.json["errors"][0].GetString());
                        return;
                    }
                    onError(0, "Internal Error");
                    return;
                }
                auto& authorization = res.json["authorization"];

                std::string token = authorization["token"].GetString();
                onResult(token);
            },
            [=](const std::string& msg) {
                onError(0, msg);
            });
    }

    static void setResultParams(const IClient::MatchResult & result, std::vector<std::pair<std::string, std::string>> * params) {
        for (size_t i = 0; i < result.rounds.size(); i++) {
            auto& round = result.rounds[i];
            std::string r("match[rounds_attributes][" + std::to_string(i) + "]");

            params->push_back(std::pair(r + "[id]", std::to_string(round.id)));
            params->push_back(std::pair(r + "[home_team_score]", std::to_string(round.homeTeamScore)));
            params->push_back(std::pair(r + "[away_team_score]", std::to_string(round.awayTeamScore)));
        }
        // TODO: Logs, etc.
    }

    void Client::updateMatch(
            uint64_t matchId,
            std::string matchToken,
            MatchResult result,
            std::function<void ()> onResult,
            ErrorCallback onError) {

        std::string url = endpoint + "/api/v1/auto_match/update";

        std::vector<std::pair<std::string, std::string>> params;
        params.push_back(std::pair("id", std::to_string(matchId)));
        params.push_back(std::pair("token", matchToken));
        setResultParams(result, &params);
        Requests::Request req(Requests::Method::POST, url, params);

        requests->request(req,
            [=](const Requests::Response& res) {
                if (res.json.HasMember("errors")) {
                    onError(res.code, res.json["errors"][0].GetString());
                    return;
                }

                onResult();
            },
            [=](const std::string& msg) {
                onError(0, msg);
            });
    }

    void Client::submitMatch(
            uint64_t matchId,
            std::string matchToken,
            MatchResult result,
            std::function<void ()> onResult,
            ErrorCallback onError) {

        std::string url = endpoint + "/api/v1/auto_match/submit";

        std::vector<std::pair<std::string, std::string>> params;
        params.push_back(std::pair("id", std::to_string(matchId)));
        params.push_back(std::pair("token", matchToken));
        setResultParams(result, &params);
        Requests::Request req(Requests::Method::POST, url, params);

        requests->request(req,
            [=](const Requests::Response& res) {
                if (res.json.HasMember("errors")) {
                    onError(res.code, res.json["errors"][0].GetString());
                    return;
                }

                onResult();
            },
            [=](const std::string& msg) {
                onError(0, msg);
            });
    }
}
