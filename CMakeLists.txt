cmake_minimum_required(VERSION 3.10)

# Project name
project(persistent_cli)

# Use modern C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Executable
add_executable(persistent_cli
    src/main.cpp
    src/conversation.cpp
)

# Include directory
target_include_directories(persistent_cli PRIVATE include)
