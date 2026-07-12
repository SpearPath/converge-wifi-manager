#include "router/ZteF670LRouterClient.hpp"

#include <regex>
#include <sstream>
#include <utility>
#include <random>

namespace converge::router {

// ponytail: all endpoint paths and HTML patterns below are based on community
// reverse-engineering of ZTE ZXHN F670L firmware (Converge PH variant).
// They WILL differ across firmware versions and ISP customizations.
// Upgrade path: make these configurable or add firmware-version detection.

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "advapi32.lib")

std::string sha256Hex(const std::string& input) {
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    std::string result;
    if (CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        if (CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
            if (CryptHashData(hHash, reinterpret_cast<const BYTE*>(input.data()), static_cast<DWORD>(input.size()), 0)) {
                DWORD hashLen = 0;
                DWORD cbHashLen = sizeof(DWORD);
                if (CryptGetHashParam(hHash, HP_HASHSIZE, reinterpret_cast<BYTE*>(&hashLen), &cbHashLen, 0)) {
                    std::vector<BYTE> hashBuffer(hashLen);
                    if (CryptGetHashParam(hHash, HP_HASHVAL, hashBuffer.data(), &hashLen, 0)) {
                        char hex[3];
                        for (BYTE b : hashBuffer) {
                            snprintf(hex, sizeof(hex), "%02x", b);
                            result += hex;
                        }
                    }
                }
            }
            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProv, 0);
    }
    return result;
}
#else
#include <openssl/evp.h>
std::string sha256Hex(const std::string& input) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int len = 0;
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, input.data(), input.size());
    EVP_DigestFinal_ex(ctx, hash, &len);
    EVP_MD_CTX_free(ctx);
    std::string result;
    char hex[3];
    for (unsigned int i = 0; i < len; ++i) {
        snprintf(hex, sizeof(hex), "%02x", hash[i]);
        result += hex;
    }
    return result;
}
#endif

namespace {

// URL-encode a string for form POST bodies
std::string urlEncode(const std::string& value) {
    std::ostringstream out;
    for (unsigned char c : value) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            out << c;
        } else {
            out << '%' << "0123456789ABCDEF"[c >> 4] << "0123456789ABCDEF"[c & 0xF];
        }
    }
    return out.str();
}

// Extract a value from an HTML input element: <input ... name="key" ... value="VAL" ...>
std::string extractInputValue(const std::string& html, const std::string& name) {
    // ponytail: regex on HTML is fragile. Ceiling: breaks on multiline attributes
    // or unusual quoting. Upgrade path: proper HTML parser.
    std::regex pattern(
        R"(<input[^>]*name\s*=\s*["'])" + name + R"(["'][^>]*value\s*=\s*["']([^"']*)["'])",
        std::regex::icase);
    std::smatch match;
    if (std::regex_search(html, match, pattern)) {
        return match[1].str();
    }
    // Try reversed attribute order (value before name)
    std::regex patternRev(
        R"(<input[^>]*value\s*=\s*["']([^"']*)["'][^>]*name\s*=\s*["'])" + name + R"(["'])",
        std::regex::icase);
    if (std::regex_search(html, match, patternRev)) {
        return match[1].str();
    }
    return {};
}

// Extract text between two markers in HTML
std::string extractBetween(const std::string& html,
                            const std::string& before,
                            const std::string& after) {
    auto pos = html.find(before);
    if (pos == std::string::npos) return {};
    pos += before.size();
    auto end = html.find(after, pos);
    if (end == std::string::npos) return {};
    return html.substr(pos, end - pos);
}

}  // namespace

ZteF670LRouterClient::ZteF670LRouterClient(models::AppConfig config,
                                             network::IHttpClient& httpClient)
    : config_(std::move(config)),
      httpClient_(httpClient) {}

std::string ZteF670LRouterClient::baseUrl() const {
    return "http://" + config_.routerIp;
}

void ZteF670LRouterClient::mergeCookies(const network::HttpResponse& response) {
    for (const auto& [name, value] : response.cookies) {
        cookies_[name] = value;
    }
}

void ZteF670LRouterClient::checkSessionExpiration(const network::HttpResponse& resp) {
    if (loggedIn_) {
        bool isRedirectToLogin = resp.headers.count("Location") && resp.headers.at("Location").find("login") != std::string::npos;
        bool bodyContainsLogin = resp.body.find("Frm_Logintoken") != std::string::npos;
        
        if (resp.statusCode == 401 || isRedirectToLogin || bodyContainsLogin) {
            loggedIn_ = false;
            sessionToken_.clear();
            cookies_.clear();
        }
    }
}

network::HttpResponse ZteF670LRouterClient::httpGet(const std::string& path) {
    network::HttpRequest req;
    req.method = network::HttpMethod::Get;
    req.url = baseUrl() + path;
    req.cookies = cookies_;
    req.headers["Referer"] = baseUrl() + "/";
    auto resp = httpClient_.send(req);
    mergeCookies(resp);
    checkSessionExpiration(resp);
    return resp;
}

network::HttpResponse ZteF670LRouterClient::httpPost(const std::string& path,
                                                       const std::string& body,
                                                       const std::string& contentType) {
    network::HttpRequest req;
    req.method = network::HttpMethod::Post;
    req.url = baseUrl() + path;
    req.body = body;
    req.cookies = cookies_;
    req.headers["Content-Type"] = contentType;
    req.headers["Referer"] = baseUrl() + "/";
    auto resp = httpClient_.send(req);
    mergeCookies(resp);
    checkSessionExpiration(resp);
    return resp;
}

std::string ZteF670LRouterClient::fetchLoginToken() {
    auto resp = httpGet("/");
    if (!resp.error.empty()) return {};
    // ponytail: token field name varies by firmware. Common names:
    // Frm_Logintoken, Frm_Loginchecktoken, _sessionTOKEN
    auto token = extractInputValue(resp.body, "Frm_Logintoken");
    if (token.empty()) token = extractInputValue(resp.body, "Frm_Loginchecktoken");
    if (token.empty()) token = extractInputValue(resp.body, "_sessionTOKEN");
    return token;
}

// ---- IRouterClient implementation ----

/**
 * @brief Authenticates with the ZTE F670L router web interface.
 * 
 * This method performs the necessary steps to log into the router, including
 * fetching CSRF tokens, hashing the password with a random number, and posting
 * the credentials. If successful, it stores the session cookies.
 * 
 * @param username The router admin username (e.g., 'admin').
 * @param password The router admin password.
 * @return models::OperationResult Contains success status and an optional error message.
 */
models::OperationResult ZteF670LRouterClient::login(const std::string& username,
                                                      const std::string& password) {
    if (username.empty() || password.empty()) {
        return models::OperationResult::failure("Username and password are required.");
    }

    // Step 1: GET login page to obtain CSRF tokens and initial cookies
    auto getResp = httpGet("/");
    if (!getResp.error.empty()) {
        loggedIn_ = false;
        return models::OperationResult::failure("Failed to fetch login page: " + getResp.error);
    }
    std::string html = getResp.body;

    // Extract Frm_Logintoken
    std::string loginToken = extractInputValue(html, "Frm_Logintoken");
    if (loginToken.empty()) {
        std::regex pattern("createHiddenInput\\(\"Frm_Logintoken\",\\s*\"([^\"]+)\"\\)");
        std::smatch match;
        if (std::regex_search(html, match, pattern)) loginToken = match[1].str();
    }

    // Extract Frm_Loginchecktoken
    std::string checkToken = extractInputValue(html, "Frm_Loginchecktoken");
    if (checkToken.empty()) {
        std::regex pattern("createHiddenInput\\(\"Frm_Loginchecktoken\",\\s*\"([^\"]+)\"\\)");
        std::smatch match;
        if (std::regex_search(html, match, pattern)) checkToken = match[1].str();
    }

    // Generate random 8-digit number (Math.round(Math.random()*89999999)+10000000)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(10000000, 99999999);
    std::string randomNumStr = std::to_string(dist(gen));

    // Compute SHA256 of password + randomNum
    std::string hashedPassword = sha256Hex(password + randomNumStr);

    // Step 2: POST credentials
    std::string body = "action=login"
                     + std::string("&Username=") + urlEncode(username)
                     + "&Password=" + urlEncode(hashedPassword)
                     + "&UserRandomNum=" + urlEncode(randomNumStr);
    if (!loginToken.empty()) {
        body += "&Frm_Logintoken=" + urlEncode(loginToken);
    }
    if (!checkToken.empty()) {
        body += "&Frm_Loginchecktoken=" + urlEncode(checkToken);
    }

    auto resp = httpPost("/", body);

    if (!resp.error.empty()) {
        loggedIn_ = false;
        return models::OperationResult::failure("Login request failed: " + resp.error);
    }

    // Check for success: redirect (302) to a dashboard page, or 200 with no login form
    bool isRedirect = (resp.statusCode == 301 || resp.statusCode == 302);
    bool bodyHasNoLoginForm = resp.body.find("Frm_Logintoken") == std::string::npos
                           && resp.body.find("Username") == std::string::npos;

    if (isRedirect || (resp.statusCode == 200 && bodyHasNoLoginForm)) {
        loggedIn_ = true;
        // Follow redirect to capture session cookies from dashboard
        if (isRedirect && resp.headers.count("Location")) {
            httpGet(resp.headers.at("Location"));
        }
        return models::OperationResult::success("Logged in successfully.");
    }

    loggedIn_ = false;
    return models::OperationResult::failure(
        "Login failed. Check your credentials. (HTTP " + std::to_string(resp.statusCode) + ")");
}

models::OperationResult ZteF670LRouterClient::logout() {
    if (loggedIn_) {
        // ponytail: logout endpoint varies. Common: /getpage.gch?pid=1001&nextpage=logoff.gch
        httpGet("/getpage.gch?pid=1001&nextpage=logoff.gch");
    }
    loggedIn_ = false;
    cookies_.clear();
    sessionToken_.clear();
    return models::OperationResult::success("Logged out.");
}

models::RouterInfo ZteF670LRouterClient::routerInfo() {
    models::RouterInfo info;
    info.model = "ZTE F670L";

    if (!loggedIn_) {
        info.wanStatus = "Not logged in";
        return info;
    }

    // Helper to unescape HTML entities like &#86;
    auto unescapeHtmlEntities = [](const std::string& str) {
        std::string res;
        for (size_t i = 0; i < str.size();) {
            if (str[i] == '&' && i + 2 < str.size() && str[i+1] == '#') {
                auto semi = str.find(';', i + 2);
                if (semi != std::string::npos && semi - i <= 6) {
                    try {
                        int code = std::stoi(str.substr(i + 2, semi - i - 2));
                        res += static_cast<char>(code);
                        i = semi + 1;
                        continue;
                    } catch (...) {}
                }
            }
            res += str[i++];
        }
        return res;
    };

    auto resp = httpGet("/getpage.gch?pid=1002&nextpage=status_dev_info_t.gch");
    if (!loggedIn_) {
        info.wanStatus = "Session expired";
        return info;
    }
    
    if (resp.error.empty() && !resp.body.empty()) {
        auto fw = extractBetween(resp.body, "id=\"Frm_SoftwareVer\" name=\"Frm_SoftwareVer\" class=\"tdright\">", "</td>");
        if (!fw.empty()) {
            info.firmwareVersion = unescapeHtmlEntities(fw);
        }

        auto uptime = extractBetween(resp.body, "id=\"Frm_UpTime\" name=\"Frm_UpTime\" class=\"tdright\">", "</td>");
        if (!uptime.empty()) {
            info.uptime = uptime;
        }
        info.wanStatus = "Session active";
    }

    return info;
}

std::vector<models::Device> ZteF670LRouterClient::connectedDevices() {
    std::vector<models::Device> devices;

    if (!loggedIn_) return devices;

    // ponytail: DHCP client list page. Common paths:
    // /getpage.gch?pid=1005&nextpage=net_dhcp_dynamic_t.gch
    // /status_deviceinfo.html (some firmware puts it here)
    auto resp = httpGet("/getpage.gch?pid=1002&nextpage=net_dhcp_dynamic_t.gch");
    
    // Check if session expired during the request
    if (!loggedIn_) return devices;
    
    if (!resp.error.empty() || resp.body.empty()) return devices;

    // Helper to unescape ZTE hex string e.g., "\x3a" -> ":", "\x2e" -> "."
    auto unescapeZteStr = [](const std::string& str) {
        std::string res;
        for (size_t i = 0; i < str.size();) {
            if (str[i] == '\\' && i + 3 < str.size() && str[i+1] == 'x') {
                std::string hex = str.substr(i+2, 2);
                try {
                    res += static_cast<char>(std::stoi(hex, nullptr, 16));
                    i += 4;
                } catch (...) {
                    res += str[i++];
                }
            } else {
                res += str[i++];
            }
        }
        return res;
    };

    // The ZTE F670L stores DHCP data in script tags like:
    // Transfer_meaning('MACAddr0','fe\x3a5e\x3a45...');
    // Transfer_meaning('IPAddr0','192\x2e168...');
    // Transfer_meaning('HostName0','My\x2dPC');
    
    std::map<int, models::Device> deviceMap;
    std::regex pattern("Transfer_meaning\\('([A-Za-z]+)(\\d+)','([^']*)'\\);");
    
    auto words_begin = std::sregex_iterator(resp.body.begin(), resp.body.end(), pattern);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string key = match[1].str();
        int index = std::stoi(match[2].str());
        std::string val = unescapeZteStr(match[3].str());

        if (key == "MACAddr") {
            deviceMap[index].macAddress = val;
        } else if (key == "IPAddr") {
            deviceMap[index].ipAddress = val;
        } else if (key == "HostName") {
            deviceMap[index].name = val;
            if (val.empty()) deviceMap[index].name = "Unknown";
        }
    }

    for (const auto& [idx, dev] : deviceMap) {
        if (!dev.macAddress.empty() && !dev.ipAddress.empty()) {
            devices.push_back(dev);
        }
    }

    return devices;
}

models::OperationResult ZteF670LRouterClient::blockDevice(const std::string& macAddress) {
    if (macAddress.empty()) {
        return models::OperationResult::failure("MAC address is required.");
    }
    if (!loggedIn_) {
        return models::OperationResult::failure("Not logged in. Login first.");
    }

    // ponytail: MAC filter endpoint varies significantly across firmware.
    // Common approach: POST to WLAN access control page.
    // /getpage.gch?pid=1002&nextpage=net_wlanm_macfilter1_t.gch
    // Fields: WMACFilter=Enable, WFilterMode=Block, WMACAddr=XX:XX:XX:XX:XX:XX
    std::string body = "WMACFilter=Enable"
                       "&WFilterMode=Block"
                       "&WMACAddr=" + urlEncode(macAddress);
    auto resp = httpPost("/getpage.gch?pid=1002&nextpage=net_wlanm_macfilter1_t.gch", body);

    if (!loggedIn_) {
        return models::OperationResult::failure("Session expired during block request. Login again.");
    }

    if (!resp.error.empty()) {
        return models::OperationResult::failure("Block request failed: " + resp.error);
    }
    if (resp.statusCode >= 200 && resp.statusCode < 400) {
        return models::OperationResult::success("Block request sent for " + macAddress
            + ". Verify on router admin page.");
    }
    return models::OperationResult::failure(
        "Block may have failed (HTTP " + std::to_string(resp.statusCode) + "). Check router admin page.");
}

models::OperationResult ZteF670LRouterClient::unblockDevice(const std::string& macAddress) {
    if (macAddress.empty()) {
        return models::OperationResult::failure("MAC address is required.");
    }
    if (!loggedIn_) {
        return models::OperationResult::failure("Not logged in. Login first.");
    }

    // ponytail: unblock = remove MAC from filter list or set mode to Allow.
    // Exact mechanism is firmware-dependent.
    std::string body = "WMACFilter=Enable"
                       "&WFilterMode=Allow"
                       "&WMACAddr=" + urlEncode(macAddress);
    auto resp = httpPost("/getpage.gch?pid=1002&nextpage=net_wlanm_macfilter1_t.gch", body);

    if (!loggedIn_) {
        return models::OperationResult::failure("Session expired during unblock request. Login again.");
    }

    if (!resp.error.empty()) {
        return models::OperationResult::failure("Unblock request failed: " + resp.error);
    }
    if (resp.statusCode >= 200 && resp.statusCode < 400) {
        return models::OperationResult::success("Unblock request sent for " + macAddress
            + ". Verify on router admin page.");
    }
    return models::OperationResult::failure(
        "Unblock may have failed (HTTP " + std::to_string(resp.statusCode) + "). Check router admin page.");
}

}  // namespace converge::router
