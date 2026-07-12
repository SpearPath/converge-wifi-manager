cmake_minimum_required(VERSION 3.20)

project(ConvergeWiFiManager VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
    set(NETWORK_CLIENT src/network/WinHttpClient.cpp)
else()
    set(NETWORK_CLIENT src/network/CurlHttpClient.cpp)
endif()

add_executable(converge-wifi-manager
    src/cli/CommandDispatcher.cpp
    src/core/Application.cpp
    ${NETWORK_CLIENT}
    src/router/ZteF670LRouterClient.cpp
    src/services/DeviceService.cpp
    src/utils/ConfigLoader.cpp
    src/utils/Logger.cpp
    src/main.cpp
)

target_include_directories(converge-wifi-manager PRIVATE include)

if(MSVC)
    target_compile_options(converge-wifi-manager PRIVATE /W4)
else()
    target_compile_options(converge-wifi-manager PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32)
    target_link_libraries(converge-wifi-manager PRIVATE winhttp)
else()
    find_package(CURL REQUIRED)
    target_link_libraries(converge-wifi-manager PRIVATE CURL::libcurl)
endif()
