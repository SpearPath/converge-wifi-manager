#pragma once

#include <string>
#include <vector>

#include "models/Device.hpp"
#include "models/OperationResult.hpp"
#include "models/RouterInfo.hpp"

namespace converge::router {

class IRouterClient {
public:
    virtual ~IRouterClient() = default;

    virtual models::OperationResult login(const std::string& username, const std::string& password) = 0;
    virtual models::OperationResult logout() = 0;
    virtual models::RouterInfo routerInfo() = 0;
    virtual std::vector<models::Device> connectedDevices() = 0;
    virtual models::OperationResult blockDevice(const std::string& macAddress) = 0;
    virtual models::OperationResult unblockDevice(const std::string& macAddress) = 0;
};

}  // namespace converge::router
