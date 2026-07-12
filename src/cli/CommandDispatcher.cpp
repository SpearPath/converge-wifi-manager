#include "cli/CommandDispatcher.hpp"

#include <iostream>
#include <limits>

namespace converge::cli {

CommandDispatcher::CommandDispatcher(core::Application& app)
    : app_(app) {}

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
                std::cout << "Goodbye.\n";
                return;
            default:
                std::cout << "Unknown option. Try again.\n";
                pause();
                break;
        }
    }
}

void CommandDispatcher::printMenu() const {
    std::cout << "\n\033[1;36m┌─────────────────────────────────────────┐\033[0m\n"
              << "\033[1;36m│          Converge WiFi Manager          │\033[0m\n"
              << "\033[1;36m└─────────────────────────────────────────┘\033[0m\n\n"
              << "  \033[1;32m1.\033[0m Login\n"
              << "  \033[1;32m2.\033[0m Router Information\n"
              << "  \033[1;32m3.\033[0m Connected Devices\n"
              << "  \033[1;32m4.\033[0m Search Device\n"
              << "  \033[1;32m5.\033[0m Refresh Devices\n"
              << "  \033[1;32m6.\033[0m Block Device\n"
              << "  \033[1;32m7.\033[0m Unblock Device\n"
              << "  \033[1;32m8.\033[0m Settings\n"
              << "  \033[1;32m9.\033[0m Logout\n"
              << "  \033[1;31m0.\033[0m Exit\n\n"
              << "\033[1;33mChoose an option: \033[0m";
}

void CommandDispatcher::pause() const {
    std::cout << "\n\033[90mPress Enter to continue...\033[0m";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

}  // namespace converge::cli

