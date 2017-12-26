cmake_minimum_required(VERSION 2.8)

add_executable(parser-test main.cpp)

add_test(
  NAME parser
  COMMAND do_test.sh
  )
 
