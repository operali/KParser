cmake_minimum_required(VERSION 3.10)
PROJECT(libkparser)

file(GLOB_RECURSE SOURCE
  ./*.cpp
)

add_library(libkparser STATIC ${SOURCE})
