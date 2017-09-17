cmake_minimum_required(VERSION 3.0)
project(gpt-manipulator C)

include_directories(
  include
)

set(SOURCE_FILES
  src/gpt-manipulator.h
  src/gpt-manipulator.c
)

add_library(gpt-manipulator ${SOURCE_FILES})

install(TARGETS gpt-manipulator DESTINATION lib)
install(FILES src/gpt-manipulator.h DESTINATION include)
