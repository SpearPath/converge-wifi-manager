#pragma once

#include <chrono>
#include <string>

namespace converge::models {

struct AppConfig {
    std::string routerIp = "192.168.1.1";
    std::string username = "admin";
    std::chrono::seconds refreshInterval{30};
    bool autoLogin = false;
};

}  // namespace converge::models
