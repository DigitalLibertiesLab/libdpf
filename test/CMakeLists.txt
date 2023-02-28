cmake_minimum_required(VERSION 3.10)
project(libdpf_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(googletest)
enable_testing()

add_executable(
  hello_test
  hello_test.cc
)

target_link_libraries(
  hello_test
  gtest
  gtest_main
  pthread
)

include(GoogleTest)
gtest_discover_tests(hello_test)
