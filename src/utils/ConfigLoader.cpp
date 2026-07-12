#include "utils/ConfigLoader.hpp"

#include <fstream>
#include <regex>
#include <sstream>

namespace converge::utils {

namespace {

std::string readFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) {
        return {};
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string readStringValue(const std::string& json, const std::string& key, std::string fallback) {
    const std::regex pattern("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
    std::smatch match;
    return std::regex_search(json, match, pattern) ? match[1].str() : fallback;
}

bool readBoolValue(const std::string& json, const std::string& key, bool fallback) {
    const std::regex pattern("\"" + key + R"("\s*:\s*(true|false))");
    std::smatch match;
    return std::regex_search(json, match, pattern) ? match[1].str() == "true" : fallback;
}

int readIntValue(const std::string& json, const std::string& key, int fallback) {
    const std::regex pattern("\"" + key + R"("\s*:\s*([0-9]+))");
    std::smatch match;
    return std::regex_search(json, match, pattern) ? std::stoi(match[1].str()) : fallback;
}

}  // namespace

models::AppConfig ConfigLoader::load(const std::filesystem::path& path) const {
    models::AppConfig config;
    const std::string json = readFile(path);
    if (json.empty()) {
        return config;
    }

    config.routerIp = readStringValue(json, "router_ip", config.routerIp);
    config.username = readStringValue(json, "username", config.username);
    config.refreshInterval = std::chrono::seconds{
        readIntValue(json, "refresh_interval", static_cast<int>(config.refreshInterval.count()))};
    config.autoLogin = readBoolValue(json, "auto_login", config.autoLogin);
    return config;
}

}  // namespace converge::utils
