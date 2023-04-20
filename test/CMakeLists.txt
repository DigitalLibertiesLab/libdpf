cmake_minimum_required(VERSION 3.10)
project(libdpf_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(googletest)
enable_testing()

include_directories(../include ../thirdparty ../thirdparty/asio/asio/include)
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

add_executable(all_test tests/all_test.cpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
add_executable(dpf_key_test tests/dpf_key_test.cpp)
add_executable(eval_point_test tests/eval_point_test.cpp)
add_executable(eval_interval_test tests/eval_interval_test.cpp)
add_executable(eval_sequence_test tests/eval_sequence_test.cpp)

# add_executable(value_param_test tests/value_param_test.cpp)
# add_executable(type_param_test tests/type_param_test.cpp)

include(GoogleTest)
gtest_discover_tests(dpf_key_test)
gtest_discover_tests(eval_point_test)
gtest_discover_tests(eval_interval_test)
gtest_discover_tests(eval_sequence_test)

# gtest_discover_tests(value_param_test)
# gtest_discover_tests(type_param_test)
