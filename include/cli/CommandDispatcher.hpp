#pragma once

#include "core/Application.hpp"

namespace converge::cli {

class CommandDispatcher {
public:
    explicit CommandDispatcher(core::Application& app);

    void run();

private:
    void printMenu() const;
    void pause() const;

    core::Application& app_;
};

}  // namespace converge::cli
