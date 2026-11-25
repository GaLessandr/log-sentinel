cmake_minimum_required(VERSION 3.16)
project(log_sentinel VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

include_directories(include)

add_executable(log_sentinel
    src/main.cpp
    src/log_parser.cpp
    src/analyzer.cpp
)
