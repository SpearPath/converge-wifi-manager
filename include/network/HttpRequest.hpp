#pragma once

#include <map>
#include <string>

namespace converge::network {

enum class HttpMethod {
    Get,
    Post
};

struct HttpRequest {
    HttpMethod method = HttpMethod::Get;
    std::string url;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> cookies;
    std::string body;
    int timeoutSeconds = 10;
};

}  // namespace converge::network
