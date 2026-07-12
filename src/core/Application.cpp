#include "core/Application.hpp"

#include <iostream>
#include <limits>

namespace converge::core {

namespace {

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

void printDevices(const std::vector<models::Device>& devices) {
    if (devices.empty()) {
        std::cout << "No connected devices loaded.\n";
        return;
    }

    for (const auto& device : devices) {
        std::cout << "- " << device.name << " | "
                  << device.ipAddress << " | "
                  << device.macAddress
                  << (device.blocked ? " | blocked" : "")
                  << '\n';
    }
}

void printResult(const models::OperationResult& result) {
    std::cout << (result.ok ? "OK: " : "Error: ") << result.message << '\n';
}

}  // namespace

Application::Application(router::IRouterClient& routerClient,
                         services::DeviceService& deviceService,
                         utils::Logger& logger)
    : routerClient_(routerClient),
      deviceService_(deviceService),
      logger_(logger) {}

void Application::login() {
    const std::string username = readLine("Username: ");
    const std::string password = readLine("Password: ");
    const auto result = routerClient_.login(username, password);
    printResult(result);
    if (!result.ok) {
        logger_.warning(result.message);
    }
}

void Application::logout() {
    printResult(routerClient_.logout());
}

void Application::showRouterInfo() {
    const auto info = routerClient_.routerInfo();
    std::cout << "Model: " << info.model << '\n'
              << "Firmware: " << info.firmwareVersion << '\n'
              << "Uptime: " << info.uptime << '\n'
              << "WAN status: " << info.wanStatus << '\n';
}

void Application::showConnectedDevices() {
    printDevices(deviceService_.cachedDevices());
}

void Application::searchDevices() {
    const std::string query = readLine("Search by name, IP, or MAC: ");
    printDevices(deviceService_.search(query));
}

void Application::refreshDevices() {
    const auto devices = deviceService_.refresh();
    std::cout << "Loaded " << devices.size() << " connected device(s).\n";
    printDevices(devices);
}

void Application::blockDevice() {
    const std::string macAddress = readLine("MAC address to block: ");
    printResult(routerClient_.blockDevice(macAddress));
}

void Application::unblockDevice() {
    const std::string macAddress = readLine("MAC address to unblock: ");
    printResult(routerClient_.unblockDevice(macAddress));
}

void Application::showSettings() const {
    std::cout << "Settings are loaded from config/config.json.\n"
              << "Passwords are not stored by default.\n";
}

}  // namespace converge::core
