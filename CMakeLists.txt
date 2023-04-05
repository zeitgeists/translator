cmake_minimum_required(VERSION 3.25)

include(CMake/HunterPackages.cmake)

# Project name and a few useful settings. Other commands can pick up the results
project(
  translator)
  # VERSION 0.1
  # LANGUAGES CXX)

# Enable solution folder support
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Append the CMake module search path so we can use our own modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/CMake)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(DCMAKE_BUILD_TYPE Debug)

# LLVM wrapper
include(CMake/LLVM.cmake)

# MSVC-specific options
if(MSVC)
    # This assumes the installed LLVM was built in Release mode
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "/ZI /Od /Ob0 /DNDEBUG" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/ZI /Od /Ob0 /DNDEBUG" CACHE STRING "" FORCE)

    if(${LLVM_USE_CRT_RELEASE} STREQUAL "MD")
        set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
    elseif(${LLVM_USE_CRT_RELEASE} STREQUAL "MT")
        set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)
    else()
        message(FATAL_ERROR "Unsupported LLVM_USE_CRT_RELEASE=${LLVM_USE_CRT_RELEASE}")
    endif()
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
)

file(GLOB all_SRCS
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(SERIALIZATION_TESTS "build tests (ON or OFF)" OFF)

    if(SERIALIZATION_TESTS)
        add_library(gtests ${all_SRCS})
        target_link_libraries(gtests PRIVATE LLVM)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()

add_executable(${PROJECT_NAME} ${all_SRCS})

# Link against LLVM
target_link_libraries(${PROJECT_NAME} PRIVATE LLVM)

# Set the plugin as the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
