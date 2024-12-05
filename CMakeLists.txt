cmake_minimum_required(VERSION 3.29)
project(final)

set(CMAKE_CXX_STANDARD 20)
# Explicitly include the current directory to locate doctest.h
include_directories(${PROJECT_SOURCE_DIR})

add_executable(final main.cpp)

