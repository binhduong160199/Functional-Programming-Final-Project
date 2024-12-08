cmake_minimum_required(VERSION 3.29)
project(final)

set(CMAKE_CXX_STANDARD 20)

# Add vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Include current directory for headers
include_directories(${PROJECT_SOURCE_DIR})

# Add the executable
add_executable(final main.cpp)
