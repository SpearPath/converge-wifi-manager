add_executable(converge-wifi-manager-tests
    LoggerTests.cpp
    ConfigLoaderTests.cpp
    DeviceServiceTests.cpp
    RouterClientTests.cpp
    HtmlParsingTests.cpp
)
target_link_libraries(converge-wifi-manager-tests PRIVATE 
    converge-wifi-manager-core 
    GTest::gtest_main
    GTest::gmock_main
    GTest::gmock
)
include(GoogleTest)
gtest_discover_tests(converge-wifi-manager-tests)
