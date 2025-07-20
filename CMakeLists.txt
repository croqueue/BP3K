cmake_minimum_required(VERSION 3.14)

project(bp3k
        VERSION 0.1
        DESCRIPTION "BP3K is a metaprogramming library that values memory conservation, random access, and freedom from exceptions"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()
add_subdirectory(tests)

add_library(bp3k INTERFACE)
target_include_directories(bp3k INTERFACE include/)
set_target_properties(bp3k PROPERTIES VERSION ${PROJECT_VERSION})
