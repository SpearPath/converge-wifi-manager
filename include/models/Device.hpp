#pragma once

#include <string>

namespace converge::models {

struct Device {
    std::string name;
    std::string ipAddress;
    std::string macAddress;
    bool blocked = false;
};

}  // namespace converge::models
