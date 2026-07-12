#pragma once

#include <string>

#include "network/IHttpClient.hpp"

namespace converge::network {

class WinHttpClient final : public IHttpClient {
public:
    HttpResponse send(const HttpRequest& request) override;

private:
    static std::string buildCookieHeader(const HttpRequest& request);
    static void parseCookies(const std::string& header, HttpResponse& response);
};

}  // namespace converge::network
