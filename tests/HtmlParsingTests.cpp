#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "router/ZteF670LRouterClient.hpp"
#include "MockHttpClient.hpp"
#include "models/AppConfig.hpp"

using namespace converge::router;
using namespace converge::network;
using namespace converge::models;
using ::testing::_;
using ::testing::Return;

namespace {

// Helper: build a mock response
HttpResponse makeResponse(int status, std::string body,
                          std::map<std::string, std::string> headers = {}) {
    HttpResponse r;
    r.statusCode = status;
    r.body = std::move(body);
    r.headers = std::move(headers);
    return r;
}

// Login redirect response (302 with Location header, triggering the follow-redirect path)
HttpResponse loginRedirect() {
    return makeResponse(302, "", {{"Location", "/index.gch"}});
}

AppConfig testConfig() {
    AppConfig c;
    c.routerIp = "192.168.1.1";
    c.routerType = "ZTE_F670L";
    return c;
}

// Set up mock for a successful login: GET login page → POST → 302 redirect → follow
// Returns the mock with 3 calls consumed.
void expectLogin(MockHttpClient& mock, const std::string& loginPageHtml) {
    EXPECT_CALL(mock, send(_))
        .WillOnce(Return(makeResponse(200, loginPageHtml)))  // GET /
        .WillOnce(Return(loginRedirect()))                   // POST /
        .WillOnce(Return(makeResponse(200, "<html>dashboard</html>"))); // follow redirect
}

}  // namespace

// --- Login token extraction ---

TEST(HtmlParsingTests, LoginExtractsTokenFromInputElement) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    std::string loginPage =
        R"(<html><body><form>
        <input type="hidden" name="Frm_Logintoken" value="abc123token">
        <input type="hidden" name="Frm_Loginchecktoken" value="check456">
        </form></body></html>)";

    EXPECT_CALL(mock, send(_))
        .WillOnce(Return(makeResponse(200, loginPage)))
        .WillOnce(Return(loginRedirect()))
        .WillOnce(Return(makeResponse(200, "<html>dashboard</html>")));

    auto result = client.login("admin", "password123");
    EXPECT_TRUE(result.ok);
}

TEST(HtmlParsingTests, LoginExtractsTokenFromCreateHiddenInput) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    std::string loginPage =
        R"(<html><body><script>
        createHiddenInput("Frm_Logintoken", "jsToken789")
        createHiddenInput("Frm_Loginchecktoken", "jsCheck012")
        </script></body></html>)";

    EXPECT_CALL(mock, send(_))
        .WillOnce(Return(makeResponse(200, loginPage)))
        .WillOnce(Return(loginRedirect()))
        .WillOnce(Return(makeResponse(200, "<html>dashboard</html>")));

    auto result = client.login("admin", "password123");
    EXPECT_TRUE(result.ok);
}

TEST(HtmlParsingTests, LoginFailsWhenLoginFormStillPresent) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    std::string loginPage =
        R"(<html><body><form>
        <input type="hidden" name="Frm_Logintoken" value="tok">
        <input name="Username" value="">
        </form></body></html>)";

    EXPECT_CALL(mock, send(_))
        .WillOnce(Return(makeResponse(200, loginPage)))
        // POST returns 200 but body still has login form → failure
        .WillOnce(Return(makeResponse(200, loginPage)));

    auto result = client.login("admin", "wrongpassword");
    EXPECT_FALSE(result.ok);
}

TEST(HtmlParsingTests, LoginRejectsEmptyCredentials) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    EXPECT_CALL(mock, send(_)).Times(0);

    auto r1 = client.login("", "password");
    EXPECT_FALSE(r1.ok);

    auto r2 = client.login("admin", "");
    EXPECT_FALSE(r2.ok);
}

// --- Device list parsing (Transfer_meaning) ---

TEST(HtmlParsingTests, ConnectedDevicesParsesDhcpList) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    std::string loginPage = R"(<html><body></body></html>)";
    EXPECT_CALL(mock, send(_))
        // login: GET, POST, follow redirect
        .WillOnce(Return(makeResponse(200, loginPage)))
        .WillOnce(Return(loginRedirect()))
        .WillOnce(Return(makeResponse(200, "")))
        // connectedDevices() GET:
        .WillOnce(Return(makeResponse(200,
            R"(<html><body>
            <script language=javascript>Transfer_meaning('MACAddr0','aa\x3abb\x3acc\x3add\x3aee\x3aff');</script>
            <script language=javascript>Transfer_meaning('IPAddr0','192\x2e168\x2e1\x2e100');</script>
            <script language=javascript>Transfer_meaning('HostName0','MyPhone');</script>
            <script language=javascript>Transfer_meaning('MACAddr1','11\x3a22\x3a33\x3a44\x3a55\x3a66');</script>
            <script language=javascript>Transfer_meaning('IPAddr1','192\x2e168\x2e1\x2e101');</script>
            <script language=javascript>Transfer_meaning('HostName1','');</script>
            </body></html>)")));

    client.login("admin", "pass");
    auto devices = client.connectedDevices();

    ASSERT_EQ(devices.size(), 2u);

    EXPECT_EQ(devices[0].macAddress, "aa:bb:cc:dd:ee:ff");
    EXPECT_EQ(devices[0].ipAddress, "192.168.1.100");
    EXPECT_EQ(devices[0].name, "MyPhone");

    EXPECT_EQ(devices[1].macAddress, "11:22:33:44:55:66");
    EXPECT_EQ(devices[1].ipAddress, "192.168.1.101");
    EXPECT_EQ(devices[1].name, "Unknown");
}

TEST(HtmlParsingTests, ConnectedDevicesReturnsEmptyOnMalformedHtml) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    std::string loginPage = R"(<html><body></body></html>)";
    EXPECT_CALL(mock, send(_))
        .WillOnce(Return(makeResponse(200, loginPage)))
        .WillOnce(Return(loginRedirect()))
        .WillOnce(Return(makeResponse(200, "")))
        .WillOnce(Return(makeResponse(200, "<html>no devices here</html>")));

    client.login("admin", "pass");
    auto devices = client.connectedDevices();
    EXPECT_TRUE(devices.empty());
}

TEST(HtmlParsingTests, ConnectedDevicesSkipsIncompleteEntries) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    std::string loginPage = R"(<html><body></body></html>)";
    EXPECT_CALL(mock, send(_))
        .WillOnce(Return(makeResponse(200, loginPage)))
        .WillOnce(Return(loginRedirect()))
        .WillOnce(Return(makeResponse(200, "")))
        // Device with MAC but no IP should be skipped
        .WillOnce(Return(makeResponse(200,
            R"(<html><body>
            <script language=javascript>Transfer_meaning('MACAddr0','aa\x3abb\x3acc\x3add\x3aee\x3aff');</script>
            <script language=javascript>Transfer_meaning('HostName0','Orphan');</script>
            </body></html>)")));

    client.login("admin", "pass");
    auto devices = client.connectedDevices();
    EXPECT_TRUE(devices.empty());
}

// --- Router info parsing ---

TEST(HtmlParsingTests, RouterInfoExtractsFirmwareAndUptime) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    std::string loginPage = R"(<html><body></body></html>)";
    std::string infoPage =
        R"(<html><body>
        <td id="Frm_SoftwareVer" name="Frm_SoftwareVer" class="tdright">
        &#86;&#49;&#46;&#49;&#46;&#50;
        </td>
        <td id="Frm_UpTime" name="Frm_UpTime" class="tdright">3 days 12:34:56</td>
        </body></html>)";

    EXPECT_CALL(mock, send(_))
        .WillOnce(Return(makeResponse(200, loginPage)))
        .WillOnce(Return(loginRedirect()))
        .WillOnce(Return(makeResponse(200, "")))
        .WillOnce(Return(makeResponse(200, infoPage)));

    client.login("admin", "pass");
    auto info = client.routerInfo();

    EXPECT_EQ(info.model, "ZTE F670L");
    EXPECT_FALSE(info.firmwareVersion.empty());
    EXPECT_NE(info.firmwareVersion, "Unknown");
    EXPECT_EQ(info.uptime, "3 days 12:34:56");
    EXPECT_EQ(info.wanStatus, "Session active");
}

TEST(HtmlParsingTests, RouterInfoReturnsDefaultsOnEmptyBody) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    std::string loginPage = R"(<html><body></body></html>)";
    EXPECT_CALL(mock, send(_))
        .WillOnce(Return(makeResponse(200, loginPage)))
        .WillOnce(Return(loginRedirect()))
        .WillOnce(Return(makeResponse(200, "")))
        .WillOnce(Return(makeResponse(200, "")));

    client.login("admin", "pass");
    auto info = client.routerInfo();

    EXPECT_EQ(info.model, "ZTE F670L");
}

// --- Session expiration ---

TEST(HtmlParsingTests, SessionExpirationDetectedOnLoginRedirect) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    std::string loginPage = R"(<html><body></body></html>)";

    EXPECT_CALL(mock, send(_))
        .WillOnce(Return(makeResponse(200, loginPage)))
        .WillOnce(Return(loginRedirect()))
        .WillOnce(Return(makeResponse(200, "")))
        // routerInfo request gets redirected to login
        .WillOnce(Return(makeResponse(302, "", {{"Location", "/login.gch"}})));

    client.login("admin", "pass");
    auto info = client.routerInfo();
    EXPECT_EQ(info.wanStatus, "Session expired");
}

// --- HTTP error handling ---

TEST(HtmlParsingTests, LoginFailsOnNetworkError) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    HttpResponse errorResp;
    errorResp.error = "Connection refused";

    EXPECT_CALL(mock, send(_))
        .WillOnce(Return(errorResp));

    auto result = client.login("admin", "pass");
    EXPECT_FALSE(result.ok);
    EXPECT_NE(result.message.find("Connection refused"), std::string::npos);
}

TEST(HtmlParsingTests, LoginSucceedsWithout302WhenNoLoginForm) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    // Some firmware returns 200 with dashboard body (no redirect)
    std::string loginPage = R"(<html><body></body></html>)";
    std::string dashboardBody = R"(<html><body><h1>Welcome</h1></body></html>)";

    EXPECT_CALL(mock, send(_))
        .WillOnce(Return(makeResponse(200, loginPage)))  // GET /
        .WillOnce(Return(makeResponse(200, dashboardBody)));  // POST / returns 200 with no login form

    auto result = client.login("admin", "pass");
    EXPECT_TRUE(result.ok);
}

TEST(HtmlParsingTests, ConnectedDevicesReturnsEmptyOnHttpError) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    std::string loginPage = R"(<html><body></body></html>)";
    HttpResponse errorResp;
    errorResp.error = "Timeout";

    EXPECT_CALL(mock, send(_))
        .WillOnce(Return(makeResponse(200, loginPage)))
        .WillOnce(Return(loginRedirect()))
        .WillOnce(Return(makeResponse(200, "")))
        .WillOnce(Return(errorResp));

    client.login("admin", "pass");
    auto devices = client.connectedDevices();
    EXPECT_TRUE(devices.empty());
}

TEST(HtmlParsingTests, RouterInfoHandlesServerError) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    std::string loginPage = R"(<html><body></body></html>)";

    EXPECT_CALL(mock, send(_))
        .WillOnce(Return(makeResponse(200, loginPage)))
        .WillOnce(Return(loginRedirect()))
        .WillOnce(Return(makeResponse(200, "")))
        .WillOnce(Return(makeResponse(500, "Internal Server Error")));

    client.login("admin", "pass");
    auto info = client.routerInfo();
    // Should not crash, should return defaults or partial info
    EXPECT_EQ(info.model, "ZTE F670L");
}

// --- Block/unblock ---

TEST(HtmlParsingTests, BlockDeviceSucceeds) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    std::string loginPage = R"(<html><body></body></html>)";

    EXPECT_CALL(mock, send(_))
        .WillOnce(Return(makeResponse(200, loginPage)))
        .WillOnce(Return(loginRedirect()))
        .WillOnce(Return(makeResponse(200, "")))
        .WillOnce(Return(makeResponse(200, "OK")));

    client.login("admin", "pass");
    auto result = client.blockDevice("AA:BB:CC:DD:EE:FF");
    EXPECT_TRUE(result.ok);
}

TEST(HtmlParsingTests, BlockDeviceFailsWhenNotLoggedIn) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    EXPECT_CALL(mock, send(_)).Times(0);

    auto result = client.blockDevice("AA:BB:CC:DD:EE:FF");
    EXPECT_FALSE(result.ok);
    EXPECT_NE(result.message.find("Not logged in"), std::string::npos);
}

TEST(HtmlParsingTests, UnblockDeviceSucceeds) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    std::string loginPage = R"(<html><body></body></html>)";

    EXPECT_CALL(mock, send(_))
        .WillOnce(Return(makeResponse(200, loginPage)))
        .WillOnce(Return(loginRedirect()))
        .WillOnce(Return(makeResponse(200, "")))
        .WillOnce(Return(makeResponse(200, "OK")));

    client.login("admin", "pass");
    auto result = client.unblockDevice("AA:BB:CC:DD:EE:FF");
    EXPECT_TRUE(result.ok);
}

TEST(HtmlParsingTests, BlockDeviceRejectsEmptyMac) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    EXPECT_CALL(mock, send(_)).Times(0);

    auto result = client.blockDevice("");
    EXPECT_FALSE(result.ok);
}

TEST(HtmlParsingTests, SessionExpirationDuringBlock) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    std::string loginPage = R"(<html><body></body></html>)";

    EXPECT_CALL(mock, send(_))
        .WillOnce(Return(makeResponse(200, loginPage)))
        .WillOnce(Return(loginRedirect()))
        .WillOnce(Return(makeResponse(200, "")))
        // block request gets 401
        .WillOnce(Return(makeResponse(401, "")));

    client.login("admin", "pass");
    auto result = client.blockDevice("AA:BB:CC:DD:EE:FF");
    EXPECT_FALSE(result.ok);
    EXPECT_NE(result.message.find("Session expired"), std::string::npos);
}
