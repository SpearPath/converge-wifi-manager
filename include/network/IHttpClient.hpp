#pragma once

#include "network/HttpRequest.hpp"
#include "network/HttpResponse.hpp"

namespace converge::network {

class IHttpClient {
public:
    virtual ~IHttpClient() = default;

    virtual HttpResponse send(const HttpRequest& request) = 0;
};

}  // namespace converge::network
