#include "core/Application.hpp"

#include <iostream>
#include <limits>
#include <iomanip>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace converge::core {

namespace {

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

std::string readPassword(const std::string& prompt) {
    std::cout << prompt;
    std::string password;
#ifdef _WIN32
    char ch;
    while (true) {
        ch = static_cast<char>(_getch());
        if (ch == '\r' || ch == '\n') {
            break;
        }
        if (ch == '\b') { // Backspace
            if (!password.empty()) {
                std::cout << "\b \b";
                password.pop_back();
            }
        } else if (ch != 0 && ch != -32) { // Skip arrows/function keys
            password.push_back(ch);
            std::cout << '*';
        }
    }
    std::cout << '\n';
#else
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    std::getline(std::cin, password);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    return password;
}

void printDevices(const std::vector<models::Device>& devices) {
    if (devices.empty()) {
        std::cout << "\033[1;33mNo connected devices loaded.\033[0m\n";
        return;
    }

    // Print Table Header
    std::cout << "\033[1;36m┌───────────────────────────┬─────────────────┬─────────────────────┬────────────┐\033[0m\n"
              << "\033[1;36m│ \033[1;37mName                      \033[1;36m│ \033[1;37mIP Address      \033[1;36m│ \033[1;37mMAC Address         \033[1;36m│ \033[1;37mStatus     \033[1;36m│\033[0m\n"
              << "\033[1;36m├───────────────────────────┼─────────────────┼─────────────────────┼────────────┤\033[0m\n";

    for (const auto& device : devices) {
        std::string name = device.name;
        if (name.length() > 25) name = name.substr(0, 22) + "...";

        std::string statusStr = device.blocked ? "\033[1;31mBlocked\033[0m" : "\033[1;32mActive\033[0m";
        // To properly align columns with colored strings (which contain invisible escape characters),
        // we print the non-colored columns with std::setw and handle the status separately.
        std::cout << "\033[1;36m│\033[0m " << std::left << std::setw(26) << name
                  << "\033[1;36m│\033[0m " << std::left << std::setw(16) << device.ipAddress
                  << "\033[1;36m│\033[0m " << std::left << std::setw(20) << device.macAddress
                  << "\033[1;36m│\033[0m " << std::left << (device.blocked ? "Blocked   " : "Active    ") << "\033[1;36m│\033[0m\n";
    }

    std::cout << "\033[1;36m└───────────────────────────┴─────────────────┴─────────────────────┴────────────┘\033[0m\n";
}

void printResult(const models::OperationResult& result) {
    if (result.ok) {
        std::cout << "\033[1;32m[SUCCESS]\033[0m " << result.message << '\n';
    } else {
        std::cout << "\033[1;31m[ERROR]\033[0m " << result.message << '\n';
    }
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
    const std::string password = readPassword("Password: ");
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
    std::cout << "\n\033[1;35mRouter Information\033[0m\n"
              << "──────────────────\n"
              << "  Model:       " << info.model << '\n'
              << "  Firmware:    " << info.firmwareVersion << '\n'
              << "  Uptime:      " << info.uptime << '\n'
              << "  WAN Status:  " << (info.wanStatus.find("active") != std::string::npos ? "\033[1;32m" : "\033[1;31m") << info.wanStatus << "\033[0m\n";
}

void Application::showConnectedDevices() {
    printDevices(deviceService_.cachedDevices());
}

void Application::searchDevices() {
    const std::string query = readLine("Search by name, IP, or MAC: ");
    printDevices(deviceService_.search(query));
}

void Application::refreshDevices() {
    std::cout << "Refreshing connected devices from router...\n";
    const auto devices = deviceService_.refresh();
    std::cout << "\033[1;32mLoaded " << devices.size() << " connected device(s).\033[0m\n";
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
    std::cout << "\n\033[1;35mSettings Details\033[0m\n"
              << "────────────────\n"
              << "  Source:    config/config.json\n"
              << "  Security:  Passwords are not stored and are requested interactively.\n";
}

}  // namespace converge::core
