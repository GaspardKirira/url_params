cmake_minimum_required(VERSION 3.20)
project(url_params LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(url_params INTERFACE)
add_library(url_params::url_params ALIAS url_params)

target_include_directories(url_params INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

if (MSVC)
  target_compile_options(url_params INTERFACE /W4 /permissive-)
else()
  target_compile_options(url_params INTERFACE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
enable_testing()

add_executable(url_params_basic_test tests/test_basic.cpp)
target_link_libraries(url_params_basic_test PRIVATE url_params::url_params)
add_test(NAME url_params.basic COMMAND url_params_basic_test)
