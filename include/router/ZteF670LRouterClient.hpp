#pragma once

#include <map>
#include <string>
#include <vector>

#include "models/AppConfig.hpp"
#include "network/IHttpClient.hpp"
#include "router/IRouterClient.hpp"

namespace converge::router {

class ZteF670LRouterClient final : public IRouterClient {
public:
    ZteF670LRouterClient(models::AppConfig config, network::IHttpClient& httpClient);

    models::OperationResult login(const std::string& username, const std::string& password) override;
    models::OperationResult logout() override;
    models::RouterInfo routerInfo() override;
    std::vector<models::Device> connectedDevices() override;
    models::OperationResult blockDevice(const std::string& macAddress) override;
    models::OperationResult unblockDevice(const std::string& macAddress) override;

private:
    std::string baseUrl() const;
    network::HttpResponse httpGet(const std::string& path);
    network::HttpResponse httpPost(const std::string& path,
                                    const std::string& body,
                                    const std::string& contentType = "application/x-www-form-urlencoded");
    std::string fetchLoginToken();
    void mergeCookies(const network::HttpResponse& response);
    void checkSessionExpiration(const network::HttpResponse& response);

    models::AppConfig config_;
    network::IHttpClient& httpClient_;
    bool loggedIn_ = false;
    std::string sessionToken_;
    std::map<std::string, std::string> cookies_;
};

}  // namespace converge::router
