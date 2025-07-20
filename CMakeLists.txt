cmake_minimum_required(VERSION 3.14)

project(bp3k
        VERSION 0.1
        DESCRIPTION "bp3k is a metaprogramming library that values memory conservation, random access, and freedom from exceptions"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# option(BUILD_TESTS "Build the unit tests" OFF)
# 
# if (BUILD_TESTS)
#   enable_testing()
#   add_subdirectory(tests)
# endif()

enable_testing()
add_subdirectory(tests)

add_library(bp3k INTERFACE)
target_include_directories(bp3k INTERFACE include/)

# set_source_files_properties(
#   include/pinkytoe/api.h
#   PROPERTIES
#   LANGUAGE CXX
# )

# target_sources(pinkytoe PRIVATE include/pinkytoe/api.h)

# target_include_directories(bp3k
#   PUBLIC
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
# )

set_target_properties(bp3k PROPERTIES VERSION ${PROJECT_VERSION})

# target_compile_options(pinkytoe PUBLIC -Wall -Werror -Wextra -pedantic)
