cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/52204f78f94d7512df1f0f3bea1d47437a2c3a58.zip
)

FetchContent_MakeAvailable(googletest)
include(GoogleTest)

add_executable(
    bitpacker_tests
    bitpacker_tests.cpp
)

add_executable(
    ibitpacker_tests
    ibitpacker_tests.cpp
)

add_executable(
    ubitpacker_tests
    ubitpacker_tests.cpp
)

target_include_directories(bitpacker_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_include_directories(ibitpacker_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_include_directories(ubitpacker_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(bitpacker_tests
    bp3k
    GTest::gtest_main
)

target_link_libraries(ibitpacker_tests
    bp3k
    GTest::gtest_main
)

target_link_libraries(ubitpacker_tests
    bp3k
    GTest::gtest_main
)

gtest_discover_tests(bitpacker_tests)
gtest_discover_tests(ibitpacker_tests)
gtest_discover_tests(ubitpacker_tests)

