#pragma once

#include <variant>
#include <functional>

#include "rapidjson/document.h"

#include "queue.h"

class Requests final {
public:
    enum class Method {
        GET,
        POST,
    };

    struct Request final {
        const Method method = Method::GET;
        const std::string url;
        const std::vector<std::pair<std::string, std::string>> params;

        Request(const Method &m, const std::string &u) : method(m), url(u) {}

        Request(
                const Method &m,
                const std::string &u,
                const std::vector<std::pair<std::string, std::string>> &v)
            : method(m), url(u), params(v) {}
    };

    struct Response final {
        uint32_t code = 200;
        std::string body;
        rapidjson::Document json;
    };

    Requests();
    ~Requests();

    void request(
        const Request &req,
        const std::function<void (const Response &res)> &onResult,
        const std::function<void (const std::string &msg)> &onError
    );

    struct RequestEvent final {
        const Request req;
        const std::function<void (const Response &res)> onResult;
        const std::function<void (const std::string &msg)> onError;

        RequestEvent(
                const Request &r,
                const std::function<void (const Response &res)> &o,
                const std::function<void (const std::string &msg)> &e)
            : req(r), onResult(o), onError(e) {}
    };

    struct Quit final {};

    using Event = std::variant<
        RequestEvent,
        Quit
    >;

private:
    Queue<Event> eventQueue;
    pthread_t threadId;

    static void *run(void *);
    void runThread();
};
