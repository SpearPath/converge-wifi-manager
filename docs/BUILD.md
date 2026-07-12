# Build Guide

## Prerequisites

| Platform | Requirements |
|----------|-------------|
| Windows  | MSVC 2022+ (C++23), CMake 3.20+ |
| Ubuntu   | GCC 13+ or Clang 17+, CMake 3.20+, `libcurl4-openssl-dev`, `libssl-dev` |
| Termux   | `clang`, `cmake`, `libcurl`, `openssl` |

GoogleTest and nlohmann/json are fetched automatically via CMake `FetchContent` — no manual install needed.

## Build

```bash
# Configure
cmake -S . -B build

# Build
cmake --build build --config Release
```

The binary is output to `build/` (or `build/Release/` on Windows with MSVC).

## Run Tests

```bash
cd build
ctest --output-on-failure -C Release
```

## Optional Build Flags

| Flag | Default | Description |
|------|---------|-------------|
| `-DENABLE_CLANG_TIDY=ON` | OFF | Run clang-tidy during build (Linux/macOS only) |
| `-DENABLE_SANITIZERS=ON` | OFF | Build with AddressSanitizer + UndefinedBehaviorSanitizer (GCC/Clang only) |
| `-DENABLE_COVERAGE=ON` | OFF | Enable code coverage instrumentation via gcov (GCC/Clang only) |

Example with all checks enabled:

```bash
cmake -S . -B build -DENABLE_CLANG_TIDY=ON -DENABLE_SANITIZERS=ON
cmake --build build
cd build && ctest --output-on-failure
```

## Configuration

Copy the example config and edit:

```bash
cp config/config.example.json config/config.json
```

See `config/config.example.json` for available fields (`router_ip`, `router_type`, `username`, `refresh_interval`, `auto_login`).
