#pragma once

#include <string>
#include <vector>

#include "models/Device.hpp"
#include "router/IRouterClient.hpp"

namespace converge::services {

class DeviceService {
public:
    explicit DeviceService(router::IRouterClient& routerClient);

    std::vector<models::Device> refresh();
    std::vector<models::Device> search(const std::string& query) const;
    const std::vector<models::Device>& cachedDevices() const;

private:
    router::IRouterClient& routerClient_;
    std::vector<models::Device> devices_;
};

}  // namespace converge::services
