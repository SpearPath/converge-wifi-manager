#pragma once

#include <gmock/gmock.h>
#include "network/IHttpClient.hpp"

namespace converge::network {

class MockHttpClient : public IHttpClient {
public:
    MOCK_METHOD(HttpResponse, send, (const HttpRequest&), (override));
};

} // namespace converge::network
