cmake_minimum_required(VERSION 2.8)
project(small_fortran_compiler)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-std=c++11")
add_executable(sfc main.cpp parser.cpp ast.cpp)

# test
if(NOT without-test)
  enable_testing()
#  include(cmake/gtest.cmake)
#  add_subdirectory(test)
endif()
