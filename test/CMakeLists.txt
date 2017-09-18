cmake_minimum_required(VERSION 3.0)
project(gpt-manipulator-test C)

include_directories(
  ../include
)

link_directories(
  ${CMAKE_BINARY_DIR}
)

link_libraries(
  gpt-manipulator
)

set(SOURCE_FILES
  main.c
)

add_executable(gpt-manipulator-test ${SOURCE_FILES})
