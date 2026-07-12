#include "router/RouterFactory.hpp"
#include "router/ZteF670LRouterClient.hpp"
#include <stdexcept>

namespace converge::router {

std::unique_ptr<IRouterClient> RouterFactory::create(const models::AppConfig& config, network::IHttpClient& httpClient) {
    if (config.routerType == "ZTE_F670L") {
        return std::make_unique<ZteF670LRouterClient>(config, httpClient);
    }
    
    // Add new router models here in the future
    throw std::invalid_argument("Unsupported router_type: " + config.routerType);
}

}  // namespace converge::router
