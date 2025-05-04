cmake_minimum_required(VERSION 3.10)

project(main)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(
  main
  main.cpp
  json/json.cpp
)

