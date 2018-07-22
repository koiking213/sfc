cmake_minimum_required(VERSION 2.8)
project(small_fortran_compiler)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-std=c++14 -g -O0")
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG_MODE")

find_package(LLVM 6.0 REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(sfc src/main.cpp src/parser.cpp src/ast.cpp src/IR_generator.cpp src/semantic_analysis.cpp src/cst.cpp)
add_library(fortio STATIC runtime/write.c)

# Find the libraries that correspond to the LLVM components
# that we wish to use
#llvm_map_components_to_libnames(llvm_libs support irreader all)
llvm_map_components_to_libnames(llvm_libs ${LLVM_TARGETS_TO_BUILD})

# Link against LLVM libraries
target_link_libraries(sfc ${llvm_libs})

# test
if(NOT without-test)
  enable_testing()
#  include(cmake/gtest.cmake)
#  add_subdirectory(test)
endif()
