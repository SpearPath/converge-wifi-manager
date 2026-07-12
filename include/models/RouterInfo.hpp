#pragma once

#include <string>

namespace converge::models {

struct RouterInfo {
    std::string model = "ZTE F670L";
    std::string firmwareVersion = "Unknown";
    std::string uptime = "Unknown";
    std::string wanStatus = "Unknown";
};

}  // namespace converge::models
