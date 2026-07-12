#pragma once

#include "core/Application.hpp"
#include "models/AppConfig.hpp"

namespace converge::cli {

class CommandDispatcher {
public:
    CommandDispatcher(core::Application& app, const models::AppConfig& config);

    void run();

private:
    void printMenu() const;
    void pause() const;

    core::Application& app_;
    const models::AppConfig& config_;
};

}  // namespace converge::cli

