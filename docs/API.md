# API Reference

This document details the public API components of the Converge WiFi Manager core library.

---

## Service Layer

### `DeviceService`
Located in: [DeviceService.hpp](file:///d:/code/cpp/projects/convergeWifiCLIDeviceManager/include/services/DeviceService.hpp)

The primary service coordinating operations on connected devices. Maintains a cache of known devices to avoid redundant router polls.

```cpp
class DeviceService {
public:
    DeviceService(std::unique_ptr<router::IRouterClient> routerClient);

    // Refresh device cache by querying the router
    models::OperationResult refreshDevices();

    // Query active cache
    std::vector<models::Device> getCachedDevices() const;

    // Search active cache by MAC address, IP, or Name (case-insensitive substring)
    std::vector<models::Device> searchDevices(const std::string& query) const;

    // Block/unblock device by MAC
    models::OperationResult blockDevice(const std::string& macAddress);
    models::OperationResult unblockDevice(const std::string& macAddress);
};
```

---

## Router Layer

### `IRouterClient`
Located in: [IRouterClient.hpp](file:///d:/code/cpp/projects/convergeWifiCLIDeviceManager/include/router/IRouterClient.hpp)

Interface interface implemented by all router models.

```cpp
class IRouterClient {
public:
    virtual ~IRouterClient() = default;

    virtual models::OperationResult login(const std::string& username, const std::string& password) = 0;
    virtual models::OperationResult logout() = 0;
    virtual models::RouterInfo routerInfo() = 0;
    virtual std::vector<models::Device> connectedDevices() = 0;
    virtual models::OperationResult blockDevice(const std::string& macAddress) = 0;
    virtual models::OperationResult unblockDevice(const std::string& macAddress) = 0;
};
```

### `RouterFactory`
Located in: [RouterFactory.hpp](file:///d:/code/cpp/projects/convergeWifiCLIDeviceManager/include/router/RouterFactory.hpp)

Instantiates appropriate client backend by config.

```cpp
class RouterFactory {
public:
    static std::unique_ptr<IRouterClient> create(
        const models::AppConfig& config,
        network::IHttpClient& httpClient);
};
```

---

## Network Layer

### `IHttpClient`
Located in: [IHttpClient.hpp](file:///d:/code/cpp/projects/convergeWifiCLIDeviceManager/include/network/IHttpClient.hpp)

Abstract backend-independent HTTP client interface.

```cpp
class IHttpClient {
public:
    virtual ~IHttpClient() = default;
    virtual HttpResponse send(const HttpRequest& request) = 0;
};
```

Available backends:
- **`WinHttpClient`**: Uses Windows WinHTTP library.
- **`CurlHttpClient`**: Uses POSIX `libcurl` library.
