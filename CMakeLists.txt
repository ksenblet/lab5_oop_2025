cmake_minimum_required(VERSION 3.10)
project(lab5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR})

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.0
    TLS_VERIFY false
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(main src/main.cpp src/dBlockAllocator.cpp)
target_include_directories(main PRIVATE include)
target_link_libraries(main)

add_executable(tests tests/test.cpp src/dBlockAllocator.cpp)
target_include_directories(tests PRIVATE include)
target_link_libraries(tests gtest gtest_main)

enable_testing()
add_test(NAME lab5_tests COMMAND tests)