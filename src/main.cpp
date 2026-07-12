#include <filesystem>
#include <iostream>

#include "cli/CommandDispatcher.hpp"
#include "core/Application.hpp"
#ifdef _WIN32
#include "network/WinHttpClient.hpp"
#else
#include "network/CurlHttpClient.hpp"
#endif

#include "router/RouterFactory.hpp"
#include "services/DeviceService.hpp"
#include "utils/ConfigLoader.hpp"
#include "utils/Logger.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
void enableAnsiSupport() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
}
#else
void enableAnsiSupport() {}
#endif

int main() {
    enableAnsiSupport();
    converge::utils::Logger logger(std::cout);
    converge::utils::ConfigLoader configLoader;
    const auto config = configLoader.load(std::filesystem::path{"config/config.json"});

#ifdef _WIN32
    converge::network::WinHttpClient httpClient;
#else
    converge::network::CurlHttpClient httpClient;
#endif
    auto routerClient = converge::router::RouterFactory::create(config, httpClient);
    converge::services::DeviceService deviceService(*routerClient);
    converge::core::Application app(*routerClient, deviceService, logger);
    converge::cli::CommandDispatcher dispatcher(app);

    dispatcher.run();
    return 0;
}
