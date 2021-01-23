cmake_minimum_required(VERSION 3.10)
PROJECT(MyParser)

set(TARGET ${CMAKE_PROJECT_NAME})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

file(GLOB_RECURSE SOURCE
  ./src/*.h
  ./src/*.cpp
  ./test/*.cpp
)

add_subdirectory(third_party/gtest)

add_executable(${TARGET} ${SOURCE})

target_include_directories(${TARGET} PUBLIC "src")
target_include_directories(${TARGET} PUBLIC "third_party/gtest")
target_link_libraries(${TARGET} PUBLIC gtest gtest_main)
