#pragma once

#ifndef _WIN32

#include "network/IHttpClient.hpp"
#include <string>
#include <map>
#include <curl/curl.h>

namespace converge::network {

class CurlHttpClient final : public IHttpClient {
public:
    CurlHttpClient();
    ~CurlHttpClient() override;

    HttpResponse get(const std::string& url, const std::map<std::string, std::string>& headers) override;
    HttpResponse post(const std::string& url, const std::string& body, const std::map<std::string, std::string>& headers) override;

private:
    HttpResponse performRequest(const std::string& url, const std::string& body, const std::map<std::string, std::string>& headers, bool isPost);
};

} // namespace converge::network

#endif // !_WIN32
