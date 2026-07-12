#include "utils/ConfigLoader.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

namespace converge::utils {

models::AppConfig ConfigLoader::load(const std::filesystem::path& path) const {
    models::AppConfig config;
    std::ifstream file(path);
    if (!file) {
        return config;
    }

    try {
        auto j = nlohmann::json::parse(file, nullptr, false);
        if (j.is_discarded()) return config;

        if (j.contains("router_ip"))        j.at("router_ip").get_to(config.routerIp);
        if (j.contains("router_type"))      j.at("router_type").get_to(config.routerType);
        if (j.contains("username"))         j.at("username").get_to(config.username);
        if (j.contains("auto_login"))       j.at("auto_login").get_to(config.autoLogin);
        if (j.contains("refresh_interval")) {
            config.refreshInterval = std::chrono::seconds{j.at("refresh_interval").get<int>()};
        }
    } catch (const nlohmann::json::exception&) {
        // Malformed JSON — return defaults
    }

    return config;
}

}  // namespace converge::utils
