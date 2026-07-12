#pragma once

#include <string>
#include <utility>

namespace converge::models {

struct OperationResult {
    bool ok = false;
    std::string message;

    static OperationResult success(std::string message = {}) {
        return {true, std::move(message)};
    }

    static OperationResult failure(std::string message) {
        return {false, std::move(message)};
    }
};

}  // namespace converge::models
