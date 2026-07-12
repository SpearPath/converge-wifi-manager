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

    HttpResponse send(const HttpRequest& request) override;

   private:
    HttpResponse performRequest(const HttpRequest& request);
    void* curl_ = nullptr;
};

}  // namespace converge::network

#endif  // !_WIN32
