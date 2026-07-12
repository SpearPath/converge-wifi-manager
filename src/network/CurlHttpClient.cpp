#ifndef _WIN32

#include "network/CurlHttpClient.hpp"
#include <iostream>

namespace converge::network {

namespace {
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    auto* str = static_cast<std::string*>(userp);
    size_t totalSize = size * nmemb;
    str->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
    auto* headers = static_cast<std::map<std::string, std::string>*>(userdata);
    size_t numbytes = size * nitems;
    std::string header(buffer, numbytes);
    
    auto colon = header.find(':');
    if (colon != std::string::npos) {
        std::string key = header.substr(0, colon);
        std::string value = header.substr(colon + 1);
        
        // Trim whitespace
        auto keyStart = key.find_first_not_of(" \t\r\n");
        if (keyStart != std::string::npos) key = key.substr(keyStart, key.find_last_not_of(" \t\r\n") - keyStart + 1);
        
        auto valStart = value.find_first_not_of(" \t\r\n");
        if (valStart != std::string::npos) value = value.substr(valStart, value.find_last_not_of(" \t\r\n") - valStart + 1);
        
        if (!key.empty()) {
            (*headers)[key] = value;
        }
    }
    
    return numbytes;
}
} // namespace

CurlHttpClient::CurlHttpClient() {
    curl_global_init(CURL_GLOBAL_ALL);
    curl_ = curl_easy_init();
}

CurlHttpClient::~CurlHttpClient() {
    if (curl_) {
        curl_easy_cleanup(static_cast<CURL*>(curl_));
    }
    curl_global_cleanup();
}

HttpResponse CurlHttpClient::send(const HttpRequest& request) {
    return performRequest(request);
}

HttpResponse CurlHttpClient::performRequest(const HttpRequest& request) {
    HttpResponse response;
    
    CURL* curl = static_cast<CURL*>(curl_);
    if (!curl) {
        response.error = "Failed to initialize curl";
        return response;
    }
    curl_easy_reset(curl);
    
    curl_easy_setopt(curl, CURLOPT_URL, request.url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response.headers);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L); // Don't automatically follow redirect, our logic handles it
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(request.timeoutSeconds));
    
    // Set headers
    struct curl_slist* chunk = nullptr;
    for (const auto& [key, value] : request.headers) {
        std::string headerStr = key + ": " + value;
        chunk = curl_slist_append(chunk, headerStr.c_str());
    }
    
    // Set cookies
    if (!request.cookies.empty()) {
        std::string cookieStr;
        for (const auto& [key, value] : request.cookies) {
            if (!cookieStr.empty()) cookieStr += "; ";
            cookieStr += key + "=" + value;
        }
        curl_easy_setopt(curl, CURLOPT_COOKIE, cookieStr.c_str());
    }
    
    if (chunk) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    }
    
    if (request.method == HttpMethod::Post) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request.body.length());
    }
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        response.error = std::string("curl_easy_perform() failed: ") + curl_easy_strerror(res);
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        response.statusCode = static_cast<int>(response_code);
    }
    
    if (chunk) {
        curl_slist_free_all(chunk);
    }
    
    return response;
}

} // namespace converge::network

#endif // !_WIN32
