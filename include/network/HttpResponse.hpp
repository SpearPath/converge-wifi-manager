#pragma once

#include <map>
#include <string>

namespace converge::network {

struct HttpResponse {
    int statusCode = 0;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> cookies;
    std::string body;
    std::string error;

    bool ok() const {
        return statusCode >= 200 && statusCode < 300 && error.empty();
    }
};

}  // namespace converge::network
