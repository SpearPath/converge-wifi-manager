cmake_minimum_required(VERSION 3.20)

project(ConvergeWiFiManager VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    endif()
endif()

if(WIN32)
    set(NETWORK_CLIENT src/network/WinHttpClient.cpp)
else()
    set(NETWORK_CLIENT src/network/CurlHttpClient.cpp)
endif()

# Create a static library for the core code so tests can link to it
add_library(converge-wifi-manager-core STATIC
    src/cli/CommandDispatcher.cpp
    src/core/Application.cpp
    ${NETWORK_CLIENT}
    src/router/ZteF670LRouterClient.cpp
    src/router/RouterFactory.cpp
    src/services/DeviceService.cpp
    src/utils/ConfigLoader.cpp
    src/utils/Logger.cpp
)
target_include_directories(converge-wifi-manager-core PUBLIC include)

if(MSVC)
    target_compile_options(converge-wifi-manager-core PRIVATE /W4)
else()
    target_compile_options(converge-wifi-manager-core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32)
    target_link_libraries(converge-wifi-manager-core PUBLIC winhttp)
else()
    find_package(CURL REQUIRED)
    target_link_libraries(converge-wifi-manager-core PUBLIC CURL::libcurl)
endif()

# Main executable
add_executable(converge-wifi-manager src/main.cpp)
target_link_libraries(converge-wifi-manager PRIVATE converge-wifi-manager-core)

# Tests
enable_testing()
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP true
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_subdirectory(tests)
