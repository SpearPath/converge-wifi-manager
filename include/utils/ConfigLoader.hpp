#pragma once

#include <filesystem>

#include "models/AppConfig.hpp"

namespace converge::utils {

class ConfigLoader {
public:
    models::AppConfig load(const std::filesystem::path& path) const;
};

}  // namespace converge::utils
