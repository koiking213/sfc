cmake_minimum_required(VERSION 2.8)
project(small_fortran_compiler)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-std=c++14 -g -O0")

find_package(LLVM 6.0 REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(sfc main.cpp parser.cpp ast.cpp IR_generator.cpp semantic_analysis.cpp cst.cpp)

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support irreader)

# Link against LLVM libraries
target_link_libraries(sfc ${llvm_libs})


# test
if(NOT without-test)
  enable_testing()
#  include(cmake/gtest.cmake)
#  add_subdirectory(test)
endif()
