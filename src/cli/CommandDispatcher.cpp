#include "cli/CommandDispatcher.hpp"

#include <iostream>
#include <limits>

namespace converge::cli {

CommandDispatcher::CommandDispatcher(core::Application& app, const models::AppConfig& config)
    : app_(app), config_(config) {}

void CommandDispatcher::run() {
    while (true) {
        printMenu();

        int choice = -1;
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::cout << '\n';
        switch (choice) {
            case 1:
                app_.login();
                pause();
                break;
            case 2:
                app_.showRouterInfo();
                pause();
                break;
            case 3:
                app_.showConnectedDevices();
                pause();
                break;
            case 4:
                app_.searchDevices();
                pause();
                break;
            case 5:
                app_.refreshDevices();
                pause();
                break;
            case 6:
                app_.blockDevice();
                pause();
                break;
            case 7:
                app_.unblockDevice();
                pause();
                break;
            case 8:
                app_.showSettings();
                pause();
                break;
            case 9:
                app_.logout();
                pause();
                break;
            case 0:
                std::cout << "\033[1;36mGoodbye.\033[0m\n";
                return;
            default:
                std::cout << "\033[1;31mUnknown option. Try again.\033[0m\n";
                pause();
                break;
        }
    }
}

void CommandDispatcher::printMenu() const {
    // Large Cyan Title
    std::cout << "\033[1;36m"
              << "  ____   ___   _   _ __     __ ___  ____    ____  _____\n"
              << " / ___| / _ \\ | \\ | |\\ \\   / /| __||  _ \\  / ___|| ____|\n"
              << "| |    | | | ||  \\| | \\ \\ / / |  _|| |_) || |  _ |  _|\n"
              << "| |___ | |_| || |\\  |  \\ V /  | |__|  _ < | |_| || |___\n"
              << " \\____| \\___/ |_| \\_|   \\_/   |____||_| \\_\\ \\____||_____|\n\033[0m\n";

    // Divider Line
    std::cout << "\033[1;36m────────────────────────────────────────────────────────────────────────\033[0m\n";

    // Side-by-side elements. 11 lines to print.
    const int linesCount = 11;
    std::string leftCol[linesCount] = {
        "       \033[1;36m     /\\\033[0m",
        "       \033[1;36m    /  \\\033[0m",
        "       \033[1;36m   / /\\ \\\033[0m",
        "       \033[1;36m  / /  \\ \\\033[0m",
        "       \033[1;36m /_/____\\_\\\033[0m",
        "       \033[1;36m [  ||||  ]\033[0m",
        "       \033[1;36m    ||||\033[0m",
        " \033[1;36mIP:\033[0m       " + config_.routerIp,
        " \033[1;36mRouter:\033[0m   " + config_.routerType,
        " \033[1;36mUsername:\033[0m " + config_.username,
        " \033[1;36mInterval:\033[0m " + std::to_string(config_.refreshInterval.count()) + "s"
    };

    std::string rightCol[linesCount] = {
        "  \033[1;36mAvailable Commands:\033[0m",
        "  ───────────────────",
        "  \033[1;36m1.\033[0m Login",
        "  \033[1;36m2.\033[0m Router Information",
        "  \033[1;36m3.\033[0m Connected Devices",
        "  \033[1;36m4.\033[0m Search Device",
        "  \033[1;36m5.\033[0m Refresh Devices",
        "  \033[1;36m6.\033[0m Block Device",
        "  \033[1;36m7.\033[0m Unblock Device",
        "  \033[1;36m8.\033[0m Settings & Details",
        "  \033[1;36m9.\033[0m Logout  |  \033[1;31m0.\033[0m Exit"
    };

    for (int i = 0; i < linesCount; ++i) {
        // Pad the left column print to keep columns aligned.
        // We'll calculate width ignoring raw ANSI coloring escapes.
        // Plaintext of leftCol[i] has length of either:
        // "            /\033[0m" (width = 14) or " Target IP: 192.168.1.1" etc. (width = ~23)
        // Let's do standard character padding by spacing:
        std::string plainTextLeft = leftCol[i];
        // Strip out ansi codes to compute print length
        size_t ansiLen = 0;
        size_t pos = 0;
        while ((pos = plainTextLeft.find("\033", pos)) != std::string::npos) {
            size_t endPos = plainTextLeft.find("m", pos);
            if (endPos != std::string::npos) {
                ansiLen += (endPos - pos + 1);
                pos = endPos + 1;
            } else {
                break;
            }
        }
        size_t printLen = plainTextLeft.length() - ansiLen;
        std::cout << leftCol[i];
        if (printLen < 28) {
            std::cout << std::string(28 - printLen, ' ');
        }
        std::cout << rightCol[i] << '\n';
    }

    std::cout << "\033[1;36m────────────────────────────────────────────────────────────────────────\033[0m\n\n"
              << "\033[1;36mconverge-wifi> \033[0m";
}

void CommandDispatcher::pause() const {
    std::cout << "\n\033[90mPress Enter to continue...\033[0m";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

}  // namespace converge::cli

