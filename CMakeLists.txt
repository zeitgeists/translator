cmake_minimum_required(VERSION 3.25)

# Project name and a few useful settings. Other commands can pick up the results
project(
  translator
  VERSION 0.1
  LANGUAGES CXX)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
)

file(GLOB all_SRCS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(SERIALIZATION_TESTS "build tests (or no)" ON)

    if(SERIALIZATION_TESTS)
        add_library(example_tests ${all_SRCS})
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()

add_executable(translator ${all_SRCS})
