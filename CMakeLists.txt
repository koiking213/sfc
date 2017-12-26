cmake_minimum_required(VERSION 2.8)
project(small_fortran_compiler)
add_executable(main main.cpp)

# test
if(NOT without-test)
  enable_testing()
#  include(cmake/gtest.cmake)
  add_subdirectory(test)
endif()
