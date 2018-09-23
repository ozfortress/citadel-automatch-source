#include "requests.h"

#include "utils.h"

#include "rapidjson/error/en.h"

#include <cstdio>

#include <curl/curl.h>

static std::string buildParams(const std::vector<std::pair<std::string, std::string>>& params, CURL *curl) {
    std::string result;

    for (auto &param : params) {
        auto valuePtr = curl_easy_escape(curl, param.second.c_str(), param.second.length());

        result += param.first + "=" + std::string(valuePtr) + "&";

        curl_free(valuePtr);
    }

    return result;
}

static std::string requestBuildURL(const Requests::Request& req, CURL *curl) {
    if (req.params.empty()) return req.url;

    return req.url + "?" + buildParams(req.params, curl);
}

Requests::Requests() {
    // There's no equivalent cleanup as we can't guarantee that curl isn't needed by anyone else
    curl_global_init(CURL_GLOBAL_DEFAULT);

    pthread_create(&threadId, nullptr, &Requests::run, this);
}

Requests::~Requests() {
    eventQueue.enqueue(Quit());

    pthread_join(threadId, nullptr);
}

void Requests::request(
        const Request& req,
        const std::function<void (const Response& res)> &onResult,
        const std::function<void (const std::string& msg)> &onError) {

    eventQueue.enqueue(RequestEvent(
        req,
        onResult,
        onError
    ));
}

static size_t curlWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    auto body = (std::string *)userdata;
    auto realSize = size * nmemb;

    printf("Received %s\n", ptr);

    *body += std::string(ptr, realSize);

    return realSize;
}

static void curlDoRequest(const Requests::RequestEvent& event) {
    CURL *curl = curl_easy_init();
    assert(curl != nullptr, "Failed curl init");

    switch (event.req.method) {
        case Requests::Method::GET:
            curl_easy_setopt(curl, CURLOPT_HTTPGET, true);
            break;
        case Requests::Method::POST:
            curl_easy_setopt(curl, CURLOPT_POST, true);
            break;
        default:
            assert(0, "Unsupported HTTP method");
    }

    std::string url;
    if (event.req.method == Requests::Method::GET) {
        url = requestBuildURL(event.req, curl);
    } else {
        url = event.req.url;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    printf("Requesting %s\n", url.c_str());

    std::string body;
    if (event.req.method == Requests::Method::POST) {
        body = buildParams(event.req.params, curl);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        printf("With Body: %s\n", body.c_str());
    }

    Requests::Response res;

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res.body);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &curlWriteCallback);

    auto result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        std::string error(curl_easy_strerror(result));

        event.onError(error);
    } else {
        res.json.Parse(res.body.c_str());

        if (res.json.HasParseError()) {
            auto code = res.json.GetParseError();
            auto name = rapidjson::GetParseError_En(code);
            event.onError(format("RapidJSON error: %s\n", name));
        } else {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.code);

            event.onResult(res);
        }
    }

    curl_easy_cleanup(curl);
}

void *Requests::run(void *ptr) {
    auto requests = (Requests *)ptr;

    requests->runThread();

    return nullptr;
}

void Requests::runThread() {
    auto running = true;

    while (running) {
        auto event = eventQueue.dequeue();

        std::visit(overloaded {
            [&](Quit&) {
                running = false;
            },
            [&](RequestEvent& event) {
                curlDoRequest(event);
            }
        }, event);
    }
}
