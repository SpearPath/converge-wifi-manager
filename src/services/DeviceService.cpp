#include "services/DeviceService.hpp"

#include <algorithm>
#include <iterator>
#include <cctype>

namespace converge::services {

namespace {

std::string lowercase(std::string value) {
    std::ranges::transform(value, value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

bool containsIgnoreCase(const std::string& value, const std::string& query) {
    return lowercase(value).find(lowercase(query)) != std::string::npos;
}

}  // namespace

DeviceService::DeviceService(router::IRouterClient& routerClient)
    : routerClient_(routerClient) {}

std::vector<models::Device> DeviceService::refresh() {
    devices_ = routerClient_.connectedDevices();
    return devices_;
}

std::vector<models::Device> DeviceService::search(const std::string& query) const {
    std::vector<models::Device> matches;
    std::ranges::copy_if(devices_, std::back_inserter(matches), [&](const models::Device& device) {
        return containsIgnoreCase(device.name, query) ||
               containsIgnoreCase(device.ipAddress, query) ||
               containsIgnoreCase(device.macAddress, query);
    });
    return matches;
}

const std::vector<models::Device>& DeviceService::cachedDevices() const {
    return devices_;
}

}  // namespace converge::services
