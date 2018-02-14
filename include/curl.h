#pragma once

#include <memory>

#include <curl/curl.h>

namespace curl {
    void init();
    void cleanup();

    struct Callback {
        CURL *curl;
    };

    void get(std::unique_ptr<Callback> callback, std::string url, struct curl_slist *header = nullptr);
}
