#pragma once

#include <iosfwd>
#include <string>

namespace converge::utils {

enum class LogLevel {
    Info,
    Warning,
    Error,
    Debug
};

class Logger {
public:
    explicit Logger(std::ostream& output);

    void info(const std::string& message) const;
    void warning(const std::string& message) const;
    void error(const std::string& message) const;
    void debug(const std::string& message) const;

private:
    void write(LogLevel level, const std::string& message) const;

    std::ostream& output_;
};

}  // namespace converge::utils
