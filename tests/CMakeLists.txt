include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  # URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB all_SRCS
    "${PROJECT_SOURCE_DIR}/tests/*.cpp"
)

add_executable(test_project ${all_SRCS})

target_link_libraries(
    test_project
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_project)
