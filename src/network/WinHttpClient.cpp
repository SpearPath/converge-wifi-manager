#include "network/WinHttpClient.hpp"

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

#include <sstream>

namespace converge::network {

namespace {

std::wstring toWide(const std::string& s) {
    if (s.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
    std::wstring out(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), out.data(), len);
    return out;
}

std::string toNarrow(const wchar_t* s, int len) {
    if (len <= 0) return {};
    int nb = WideCharToMultiByte(CP_UTF8, 0, s, len, nullptr, 0, nullptr, nullptr);
    std::string out(static_cast<size_t>(nb), '\0');
    WideCharToMultiByte(CP_UTF8, 0, s, len, out.data(), nb, nullptr, nullptr);
    return out;
}

struct UrlParts {
    std::wstring host;
    std::wstring path;
    INTERNET_PORT port = INTERNET_DEFAULT_HTTP_PORT;
};

UrlParts parseUrl(const std::string& url) {
    URL_COMPONENTS uc{};
    uc.dwStructSize = sizeof(uc);
    wchar_t hostBuf[256]{};
    wchar_t pathBuf[2048]{};
    uc.lpszHostName = hostBuf;
    uc.dwHostNameLength = 256;
    uc.lpszUrlPath = pathBuf;
    uc.dwUrlPathLength = 2048;
    auto wide = toWide(url);
    WinHttpCrackUrl(wide.c_str(), static_cast<DWORD>(wide.size()), 0, &uc);
    return {hostBuf, pathBuf, uc.nPort};
}

}  // namespace

WinHttpClient::WinHttpClient() {
    session_ = WinHttpOpen(L"ConvergeWiFiManager/1.0",
                           WINHTTP_ACCESS_TYPE_NO_PROXY,
                           WINHTTP_NO_PROXY_NAME,
                           WINHTTP_NO_PROXY_BYPASS, 0);
}

WinHttpClient::~WinHttpClient() {
    if (connection_) WinHttpCloseHandle(static_cast<HINTERNET>(connection_));
    if (session_) WinHttpCloseHandle(static_cast<HINTERNET>(session_));
}

HttpResponse WinHttpClient::send(const HttpRequest& request) {
    HttpResponse response;

    HINTERNET session = static_cast<HINTERNET>(session_);
    if (!session) {
        response.error = "WinHttpOpen failed";
        return response;
    }

    auto parts = parseUrl(request.url);
    if (!connection_ || currentHost_ != parts.host || currentPort_ != parts.port) {
        if (connection_) WinHttpCloseHandle(static_cast<HINTERNET>(connection_));
        connection_ = WinHttpConnect(session, parts.host.c_str(), parts.port, 0);
        currentHost_ = parts.host;
        currentPort_ = parts.port;
    }

    HINTERNET connection = static_cast<HINTERNET>(connection_);
    if (!connection) {
        response.error = "WinHttpConnect failed";
        return response;
    }

    const wchar_t* verb = (request.method == HttpMethod::Post) ? L"POST" : L"GET";
    HINTERNET hRequest = WinHttpOpenRequest(connection, verb, parts.path.c_str(),
                                             nullptr, WINHTTP_NO_REFERER,
                                             WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!hRequest) {
        response.error = "WinHttpOpenRequest failed";
        return response;
    }

    // Disable auto-redirects so we can capture Set-Cookie on login redirects
    DWORD optFlags = WINHTTP_DISABLE_REDIRECTS;
    WinHttpSetOption(hRequest, WINHTTP_OPTION_DISABLE_FEATURE, &optFlags, sizeof(optFlags));

    // Set timeout
    int ms = request.timeoutSeconds * 1000;
    WinHttpSetTimeouts(hRequest, ms, ms, ms, ms);

    // Add custom headers
    for (const auto& [key, value] : request.headers) {
        auto header = toWide(key + ": " + value);
        WinHttpAddRequestHeaders(hRequest, header.c_str(),
                                  static_cast<DWORD>(header.size()),
                                  WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
    }

    // Add cookies
    std::string cookieStr = buildCookieHeader(request);
    if (!cookieStr.empty()) {
        auto header = toWide("Cookie: " + cookieStr);
        WinHttpAddRequestHeaders(hRequest, header.c_str(),
                                  static_cast<DWORD>(header.size()),
                                  WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
    }

    // Send
    LPVOID bodyData = request.body.empty() ? WINHTTP_NO_REQUEST_DATA
                                           : const_cast<char*>(request.body.data());
    DWORD bodyLen = static_cast<DWORD>(request.body.size());
    BOOL sent = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                    bodyData, bodyLen, bodyLen, 0);
    if (!sent || !WinHttpReceiveResponse(hRequest, nullptr)) {
        response.error = "HTTP request failed (network error)";
        WinHttpCloseHandle(hRequest);
        return response;
    }

    // Status code
    DWORD statusCode = 0;
    DWORD size = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                         WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &size,
                         WINHTTP_NO_HEADER_INDEX);
    response.statusCode = static_cast<int>(statusCode);

    // Response headers (for Set-Cookie parsing)
    DWORD headerSize = 0;
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                         WINHTTP_HEADER_NAME_BY_INDEX, nullptr, &headerSize,
                         WINHTTP_NO_HEADER_INDEX);
    if (headerSize > 0) {
        std::wstring rawHeaders(headerSize / sizeof(wchar_t), L'\0');
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                             WINHTTP_HEADER_NAME_BY_INDEX, rawHeaders.data(), &headerSize,
                             WINHTTP_NO_HEADER_INDEX);
        auto narrowHeaders = toNarrow(rawHeaders.data(), static_cast<int>(rawHeaders.size()));
        parseCookies(narrowHeaders, response);

        // Parse Location header for redirects
        auto locPos = narrowHeaders.find("Location: ");
        if (locPos != std::string::npos) {
            auto end = narrowHeaders.find("\r\n", locPos);
            response.headers["Location"] = narrowHeaders.substr(locPos + 10,
                end != std::string::npos ? end - locPos - 10 : std::string::npos);
        }
    }

    // Read body
    std::ostringstream body;
    char buffer[4096];
    DWORD bytesRead = 0;
    while (WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        body.write(buffer, bytesRead);
        bytesRead = 0;
    }
    response.body = body.str();

    WinHttpCloseHandle(hRequest);
    return response;
}

std::string WinHttpClient::buildCookieHeader(const HttpRequest& request) {
    std::string result;
    for (const auto& [name, value] : request.cookies) {
        if (!result.empty()) result += "; ";
        result += name + "=" + value;
    }
    return result;
}

void WinHttpClient::parseCookies(const std::string& headers, HttpResponse& response) {
    // ponytail: naive line-by-line Set-Cookie parser. Upgrade path: proper RFC 6265 parser.
    const std::string prefix = "Set-Cookie: ";
    std::string::size_type pos = 0;
    while ((pos = headers.find(prefix, pos)) != std::string::npos) {
        pos += prefix.size();
        auto eol = headers.find("\r\n", pos);
        auto cookiePart = headers.substr(pos, eol != std::string::npos ? eol - pos : std::string::npos);
        // Take only the name=value part (before first ';')
        auto semi = cookiePart.find(';');
        if (semi != std::string::npos) cookiePart = cookiePart.substr(0, semi);
        auto eq = cookiePart.find('=');
        if (eq != std::string::npos) {
            response.cookies[cookiePart.substr(0, eq)] = cookiePart.substr(eq + 1);
        }
    }
}

}  // namespace converge::network

#else
// ponytail: POSIX fallback not yet implemented. Upgrade path: raw sockets or libcurl.
#include <iostream>
namespace converge::network {

WinHttpClient::WinHttpClient() {}
WinHttpClient::~WinHttpClient() {}

HttpResponse WinHttpClient::send(const HttpRequest& request) {
    HttpResponse response;
    response.error = "HTTP transport not implemented on this platform. "
                     "Build on Windows or add a POSIX HTTP backend.";
    (void)request;
    return response;
}

std::string WinHttpClient::buildCookieHeader(const HttpRequest&) { return {}; }
void WinHttpClient::parseCookies(const std::string&, HttpResponse&) {}

}  // namespace converge::network
#endif
