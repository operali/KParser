cmake_minimum_required(VERSION 3.10)
set(TARGET kparser)
PROJECT(${TARGET})

file(GLOB_RECURSE SOURCE
  ./*.h
  ./*.cpp
)
add_library(${TARGET} STATIC ${SOURCE})
