#pragma once

#include <string>

#include "network/IHttpClient.hpp"

namespace converge::network {

class WinHttpClient : public IHttpClient {
public:
    WinHttpClient();
    ~WinHttpClient() override;

    HttpResponse send(const HttpRequest& request) override;

private:
    std::string buildCookieHeader(const HttpRequest& request);
    void parseCookies(const std::string& headers, HttpResponse& response);

    void* session_ = nullptr;
    void* connection_ = nullptr;
    std::wstring currentHost_;
    int currentPort_ = 0;
};

}  // namespace converge::network
