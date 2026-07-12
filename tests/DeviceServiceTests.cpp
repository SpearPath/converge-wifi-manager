#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "services/DeviceService.hpp"
#include "router/IRouterClient.hpp"

using namespace converge::services;
using namespace converge::router;
using namespace converge::models;

class MockRouterClient : public IRouterClient {
public:
    MOCK_METHOD(OperationResult, login, (const std::string&, const std::string&), (override));
    MOCK_METHOD(OperationResult, logout, (), (override));
    MOCK_METHOD(RouterInfo, routerInfo, (), (override));
    MOCK_METHOD(std::vector<Device>, connectedDevices, (), (override));
    MOCK_METHOD(OperationResult, blockDevice, (const std::string&), (override));
    MOCK_METHOD(OperationResult, unblockDevice, (const std::string&), (override));
};

TEST(DeviceServiceTests, RefreshUpdatesCache) {
    MockRouterClient mockClient;
    
    std::vector<Device> mockDevices = {
        {"Phone", "192.168.1.10", "AA:BB:CC:DD:EE:01"},
        {"Laptop", "192.168.1.11", "AA:BB:CC:DD:EE:02"}
    };
    
    EXPECT_CALL(mockClient, connectedDevices())
        .WillOnce(testing::Return(mockDevices));
        
    DeviceService service(mockClient);
    auto devices = service.refresh();
    
    EXPECT_EQ(devices.size(), 2);
    EXPECT_EQ(service.cachedDevices().size(), 2);
    EXPECT_EQ(service.cachedDevices()[0].name, "Phone");
}

TEST(DeviceServiceTests, SearchFiltersCacheByMacOrNameOrIp) {
    MockRouterClient mockClient;
    
    std::vector<Device> mockDevices = {
        {"Desktop PC", "192.168.1.10", "AA:BB:CC:DD:EE:01"},
        {"Work Laptop", "192.168.1.11", "AA:BB:CC:DD:EE:02"}
    };
    
    EXPECT_CALL(mockClient, connectedDevices())
        .WillOnce(testing::Return(mockDevices));
        
    DeviceService service(mockClient);
    service.refresh();
    
    // Search by partial name (case insensitive)
    auto results1 = service.search("laptop");
    EXPECT_EQ(results1.size(), 1);
    EXPECT_EQ(results1[0].name, "Work Laptop");

    // Search by partial MAC
    auto results2 = service.search("AA:BB:CC");
    EXPECT_EQ(results2.size(), 2);

    // Search by partial IP
    auto results3 = service.search("1.10");
    EXPECT_EQ(results3.size(), 1);
    EXPECT_EQ(results3[0].name, "Desktop PC");
}
