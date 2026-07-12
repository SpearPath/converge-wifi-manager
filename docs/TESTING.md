# Testing Guide

This document describes how to build, run, and extend the test suite.

---

## Quick Start

Tests are built automatically unless explicitly disabled. To run the suite:

```bash
cd build
ctest --output-on-failure
```

---

## Coverage and Sanitizers

### AddressSanitizer (ASan) & UndefinedBehaviorSanitizer (UBSan)
To build and verify tests with runtime memory and UB sanitization:

```bash
cmake -S . -B build -DENABLE_SANITIZERS=ON
cmake --build build
cd build && ctest --output-on-failure
```

### Code Coverage (lcov)
To generate local HTML coverage reports:

```bash
cmake -S . -B build -DENABLE_COVERAGE=ON
cmake --build build
cd build && ctest

# Collect & generate HTML
lcov --capture --directory . --output-file coverage.info --ignore-errors mismatch
lcov --remove coverage.info '/usr/*' '*/build/_deps/*' '*/tests/*' --output-file coverage.info
genhtml coverage.info --output-directory coverage-report
```
Open `coverage-report/index.html` in a browser.

---

## Test Directory Structure

Located under [tests/](file:///d:/code/cpp/projects/convergeWifiCLIDeviceManager/tests/):

- **`LoggerTests.cpp`**: Verifies formatting and level logs.
- **`ConfigLoaderTests.cpp`**: Verifies nlohmann/json schema load fallback paths.
- **`DeviceServiceTests.cpp`**: Mocks router interface to test cache lookup logic.
- **`RouterClientTests.cpp`**: Verifies basic state guards when logged out.
- **`HtmlParsingTests.cpp`**: Exercises scraping parsing, CSRF token extraction, hex-unescaping, session expirations, and mock network exceptions.

---

## Writing New Scraping Tests

When adding features or fixing firmware compatibility, add mock HTML test fixtures in [HtmlParsingTests.cpp](file:///d:/code/cpp/projects/convergeWifiCLIDeviceManager/tests/HtmlParsingTests.cpp):

```cpp
TEST(HtmlParsingTests, MyNewFeatureScraping) {
    MockHttpClient mock;
    ZteF670LRouterClient client(testConfig(), mock);

    expectLogin(mock, "<html>login page</html>");

    EXPECT_CALL(mock, send(_))
        .WillOnce(Return(makeResponse(200, "<html>My target markup</html>")));

    client.login("admin", "pass");
    // Call client method & assert output
}
```
Use real router response dumps from the `debug_dumps/` directory as templates for your HTML mock responses.
