#include <gtest/gtest.h>
#include "utils/Logger.hpp"
#include <sstream>

using namespace converge::utils;

TEST(LoggerTests, WritesInfoMessageCorrectly) {
    std::stringstream ss;
    Logger logger(ss);
    
    logger.info("Test info message");
    
    std::string output = ss.str();
    EXPECT_NE(output.find("Test info message"), std::string::npos);
}

TEST(LoggerTests, WritesErrorMessageCorrectly) {
    std::stringstream ss;
    Logger logger(ss);
    
    logger.error("Test error message");
    
    std::string output = ss.str();
    EXPECT_NE(output.find("Test error message"), std::string::npos);
}
