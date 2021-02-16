cmake_minimum_required(VERSION 3.10)
set(TARGET test)
PROJECT(${TARGET})

file(GLOB_RECURSE SOURCE
  ./*.cpp
)

add_executable(${TARGET} ${SOURCE})
target_link_libraries(${TARGET} PUBLIC gtest gtest_main libkparser)
