cmake_minimum_required(VERSION 3.16)

project(IronBox
        VERSION 0.1.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
    ironbox
    src/main.cpp
    src/process.cpp
    src/namespace.cpp
    src/filesystem.cpp
    src/cgroup.cpp
)

target_include_directories(
    ironbox
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_options(
    ironbox
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)