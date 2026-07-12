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
    std::cout << "\n=============================\n"
              << " Converge WiFi Manager\n"
              << "=============================\n\n"
              << "1. Login\n"
              << "2. Router Information\n"
              << "3. Connected Devices\n"
              << "4. Search Device\n"
              << "5. Refresh Devices\n"
              << "6. Block Device\n"
              << "7. Unblock Device\n"
              << "8. Settings\n"
              << "9. Logout\n"
              << "0. Exit\n\n"
              << "Choose an option: ";
}

void CommandDispatcher::pause() const {
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

}  // namespace converge::cli
