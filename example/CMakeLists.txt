cmake_minimum_required(VERSION 3.10)
PROJECT(example)

file(GLOB_RECURSE SOURCE
  ./*.cpp
)

add_executable(example ${SOURCE})
target_link_libraries(example PUBLIC libkparser)