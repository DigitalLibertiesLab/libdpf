cmake_minimum_required(VERSION 3.10)
project(libdpf_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(BUILD_GMOCK "Builds the googlemock subproject" OFF)
option(INSTALL_GTEST "Enable installation of googletest" OFF)
add_subdirectory(googletest)
enable_testing()

include_directories(../include ../thirdparty ../thirdparty/asio/asio/include)
link_libraries(gtest_main bsd)
add_compile_options(-march=native)

option(COVERAGE "Compile with coverage instrumentation" OFF)
if(COVERAGE)
    add_compile_options(-g -O0 -fno-inline -fno-inline-small-functions -fno-default-inline -fprofile-arcs -ftest-coverage)
    link_libraries(gcov)
endif()

add_executable(all_test tests/all_test.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
add_executable(dpf_key_test tests/dpf_key_test.cpp)
add_executable(wildcard_test tests/wildcard_test.cpp)

add_executable(eval_point_test tests/eval_point_test.cpp)
add_executable(eval_interval_test tests/eval_interval_test.cpp)
add_executable(eval_full_test tests/eval_full_test.cpp)
add_executable(eval_sequence_test tests/eval_sequence_test.cpp)

add_executable(eval_point_multi_test tests/eval_point_multi_test.cpp)
add_executable(eval_interval_multi_test tests/eval_interval_multi_test.cpp)
add_executable(eval_full_multi_test tests/eval_full_multi_test.cpp)
add_executable(eval_sequence_multi_test tests/eval_sequence_multi_test.cpp)

add_executable(advice_bit_iterable_test tests/advice_bit_iterable_test.cpp)
add_executable(parallel_bit_iterable_test tests/parallel_bit_iterable_test.cpp)
add_executable(setbit_index_iterable_test tests/setbit_index_iterable_test.cpp)

include(GoogleTest)
gtest_discover_tests(dpf_key_test)
gtest_discover_tests(wildcard_test)

gtest_discover_tests(eval_point_test)
gtest_discover_tests(eval_interval_test)
gtest_discover_tests(eval_full_test)
gtest_discover_tests(eval_sequence_test)

gtest_discover_tests(eval_point_multi_test)
gtest_discover_tests(eval_interval_multi_test)
gtest_discover_tests(eval_full_multi_test)
gtest_discover_tests(eval_sequence_multi_test)

gtest_discover_tests(advice_bit_iterable_test)
gtest_discover_tests(parallel_bit_iterable_test)
gtest_discover_tests(setbit_index_iterable_test)
