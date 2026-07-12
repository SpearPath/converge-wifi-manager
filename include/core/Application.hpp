#pragma once

#include "router/IRouterClient.hpp"
#include "services/DeviceService.hpp"
#include "utils/Logger.hpp"

namespace converge::core {

class Application {
public:
    Application(router::IRouterClient& routerClient,
                services::DeviceService& deviceService,
                utils::Logger& logger);

    void login();
    void logout();
    void showRouterInfo();
    void showConnectedDevices();
    void searchDevices();
    void refreshDevices();
    void blockDevice();
    void unblockDevice();
    void showSettings() const;

private:
    router::IRouterClient& routerClient_;
    services::DeviceService& deviceService_;
    utils::Logger& logger_;
};

}  // namespace converge::core
