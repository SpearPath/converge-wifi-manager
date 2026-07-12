#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "router/ZteF670LRouterClient.hpp"
#include "MockHttpClient.hpp"
#include "models/AppConfig.hpp"

using namespace converge::router;
using namespace converge::network;
using namespace converge::models;

TEST(RouterClientTests, RouterInfoReturnsNotLoggedInWhenNoSession) {
    AppConfig config;
    config.routerIp = "192.168.1.1";
    MockHttpClient mockHttp;
    
    // Should not make any network calls if not logged in
    EXPECT_CALL(mockHttp, send(testing::_)).Times(0);
        
    ZteF670LRouterClient client(config, mockHttp);
    
    auto info = client.routerInfo();
    
    EXPECT_EQ(info.wanStatus, "Not logged in");
}

TEST(RouterClientTests, ConnectedDevicesReturnsEmptyWhenNotLoggedIn) {
    AppConfig config;
    MockHttpClient mockHttp;
    
    EXPECT_CALL(mockHttp, send(testing::_)).Times(0);
        
    ZteF670LRouterClient client(config, mockHttp);
    
    auto devices = client.connectedDevices();
    
    EXPECT_TRUE(devices.empty());
}
