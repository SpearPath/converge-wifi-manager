#include "utils/Logger.hpp"

#include <iostream>

namespace converge::utils {

namespace {

const char* labelFor(LogLevel level) {
    switch (level) {
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warning:
            return "WARN";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Debug:
            return "DEBUG";
    }
    return "LOG";
}

}  // namespace

Logger::Logger(std::ostream& output)
    : output_(output) {}

void Logger::info(const std::string& message) const {
    write(LogLevel::Info, message);
}

void Logger::warning(const std::string& message) const {
    write(LogLevel::Warning, message);
}

void Logger::error(const std::string& message) const {
    write(LogLevel::Error, message);
}

void Logger::debug(const std::string& message) const {
    write(LogLevel::Debug, message);
}

void Logger::write(LogLevel level, const std::string& message) const {
    output_ << '[' << labelFor(level) << "] " << message << '\n';
}

}  // namespace converge::utils
