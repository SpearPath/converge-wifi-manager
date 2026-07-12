#include <gtest/gtest.h>
#include "utils/ConfigLoader.hpp"
#include <fstream>
#include <filesystem>

using namespace converge::utils;

class ConfigLoaderTests : public ::testing::Test {
protected:
    void SetUp() override {
        testFilePath = "test_config.json";
    }

    void TearDown() override {
        if (std::filesystem::exists(testFilePath)) {
            std::filesystem::remove(testFilePath);
        }
    }

    void createTestFile(const std::string& content) {
        std::ofstream file(testFilePath);
        file << content;
        file.close();
    }

    std::filesystem::path testFilePath;
};

TEST_F(ConfigLoaderTests, LoadsValidConfigCorrectly) {
    createTestFile(R"({
        "router_ip": "192.168.1.1",
        "username": "admin",
        "refresh_interval": 30,
        "auto_login": true
    })");

    ConfigLoader loader;
    auto config = loader.load(testFilePath);

    EXPECT_EQ(config.routerIp, "192.168.1.1");
    EXPECT_EQ(config.username, "admin");
    EXPECT_EQ(config.refreshInterval.count(), 30);
    EXPECT_TRUE(config.autoLogin);
}

TEST_F(ConfigLoaderTests, HandlesMissingValuesGracefully) {
    createTestFile(R"({
        "router_ip": "10.0.0.1"
    })");

    ConfigLoader loader;
    auto config = loader.load(testFilePath);

    // Should load the provided value
    EXPECT_EQ(config.routerIp, "10.0.0.1");
    
    // Other values should remain their defaults (we just ensure it doesn't crash)
    SUCCEED();
}
