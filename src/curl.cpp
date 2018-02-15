#include "curl.h"
#include "queue.h"
#include "utils.h"

#include <thread>

#include <curl/multi.h>

#define MAX_CONNECTIONS 3

struct Event {
    enum class Type {
        exit,
        request
    };

    Type type;
    CURL *curl;

    Event(Type type, CURL *curl = nullptr) : type(type), curl(curl) {}

    ~Event() {}
};

CURLM *curlMulti;
std::thread executorThread;
Queue<std::unique_ptr<Event>> eventQueue;

static inline std::string curlMultiErrorMessage(CURLMcode errorCode) {
    switch (errorCode) {
        case CURLM_BAD_HANDLE:
            return "CURL: Passed in handle is not a valid multi-curl handle";
        case CURLM_BAD_EASY_HANDLE:
            return "CURL: An easy handle was not valid";
        case CURLM_OUT_OF_MEMORY:
            return "CURL: Out of memory...";
        case CURLM_INTERNAL_ERROR:
            return "CURL: Internal Error!";
        case CURLM_BAD_SOCKET:
            return "CURL: Bad socket";
        case CURLM_UNKNOWN_OPTION:
            return "CURL: Unknown option was passed to multi-curl";
        case CURLM_ADDED_ALREADY:
            return "CURL: handle was already added";
        default:
            return format("CURL: Unknown CURLM error code '%d'", errorCode);
    }
}

static inline std::string curlEasyErrorMessage(CURLcode errorCode) {
    return format("CURL: Unknown easy curl error code '%d'", errorCode);
}

static void curlThread() {
    int pending = 0; // How many requests we have pending

    while (true) {
        if (pending == 0 || !eventQueue.empty()) {
            auto event = eventQueue.dequeue();

            if (event->type == Event::Type::request) {
                auto error = curl_multi_add_handle(curlMulti, event->curl);

                assert(error == CURLM_OK, curlMultiErrorMessage(error));
            } else if (event->type == Event::Type::exit) {
                break;
            } else {
                assert(0, format("Invalid event type %d", event->type));
            }
        }

        auto result = curl_multi_perform(curlMulti, &pending);
        assert(result == CURLM_OK, curlMultiErrorMessage(result));
    }
}

void curl::init() {
    curl_global_init(CURL_GLOBAL_ALL);

    curlMulti = curl_multi_init();

    curl_multi_setopt(curlMulti, CURLMOPT_MAXCONNECTS, MAX_CONNECTIONS);

    executorThread = std::thread(&curlThread);
}

void curl::cleanup() {
    std::unique_ptr<Event> event(new Event(Event::Type::exit));
    eventQueue.enqueue(std::move(event));

    curl_multi_cleanup(curlMulti);
    curl_global_cleanup();
}

template<typename ... Args>
static inline void curlEasySetOpt(CURL *curl, CURLoption info, Args ... args) {
    auto result = curl_easy_setopt(curl, info, args...);
    assert(result == CURLE_OK, curlEasyErrorMessage(result));
}

void curl::get(std::unique_ptr<Callback> callback, std::string url, struct curl_slist *header) {
    auto curl = curl_easy_init();
    assert(curl != nullptr, "Failed to initialize a easy curl instance");

    curlEasySetOpt(curl, CURLOPT_URL, url.c_str());
    if (header != nullptr) curlEasySetOpt(curl, CURLOPT_HTTPHEADER, header);

    std::unique_ptr<Event> event(new Event(Event::Type::request, std::move(curl)));
    eventQueue.enqueue(std::move(event));
}
