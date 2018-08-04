#include "requests.h"

#include "utils.h"

#include <cstdio>

#include <curl/curl.h>

static std::string requestBuildURL(const Requests::Request &req, CURL *curl) {
    std::string url = req.url + "?";

    for (auto &param : req.params) {
        auto valuePtr = curl_easy_escape(curl, param.second.c_str(), param.second.length());

        url += param.first + "=" + std::string(valuePtr);

        curl_free(valuePtr);
    }

    return url;
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
        const Request &req,
        const std::function<void (const Response &res)> &onResult,
        const std::function<void (const std::string &msg)> &onError) {

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

static void curlDoRequest(const Requests::RequestEvent &event) {
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

    const auto url = requestBuildURL(event.req, curl);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    printf("Requesting %s\n", url.c_str());

    Requests::Response res;

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res.body);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &curlWriteCallback);

    auto result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        std::string error(curl_easy_strerror(result));

        event.onError(error);
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.code);

    printf("Completed Request with %d %s\n", res.code, res.body.c_str());

    event.onResult(res);

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
