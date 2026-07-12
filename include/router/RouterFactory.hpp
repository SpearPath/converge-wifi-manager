#pragma once

#include <memory>
#include "models/AppConfig.hpp"
#include "network/IHttpClient.hpp"
#include "router/IRouterClient.hpp"

namespace converge::router {

class RouterFactory {
public:
    static std::unique_ptr<IRouterClient> create(const models::AppConfig& config, network::IHttpClient& httpClient);
};

}  // namespace converge::router
