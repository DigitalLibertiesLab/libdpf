cmake_minimum_required(VERSION 3.10)
project(libdpf_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(googletest)
enable_testing()

include_directories(../include ../thirdparty)
link_libraries(gtest_main bsd)
add_compile_options(-march=native)

option(IGNORE_ATTRIBUTES "Compile with -Wno-ignored-attributes" ON)
if (IGNORE_ATTRIBUTES)
    add_compile_options(-Wno-ignored-attributes)
endif()

option(COVERAGE "Compile with coverage instrumentation" OFF)
if(COVERAGE)
    add_compile_options(-g -O0 -fno-inline -fno-inline-small-functions -fno-default-inline -fprofile-arcs -ftest-coverage)
    link_libraries(gcov)
endif()

add_executable(dpf_key_test tests/dpf_key_test.cpp)
add_executable(eval_point_test tests/eval_point_test.cpp)

include(GoogleTest)
gtest_discover_tests(dpf_key_test)
gtest_discover_tests(eval_point_test)
