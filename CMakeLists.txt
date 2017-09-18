cmake_minimum_required(VERSION 3.0)
project(gpt-manipulator C)

add_subdirectory(test)

include_directories(
  include
)

set(SOURCE_FILES
  src/gpt-manipulator.h
  src/gpt-manipulator.c
  src/crc32.h
  src/crc32.c
)

add_library(gpt-manipulator ${SOURCE_FILES})

install(TARGETS gpt-manipulator DESTINATION lib)
install(FILES src/gpt-manipulator.h DESTINATION include)
