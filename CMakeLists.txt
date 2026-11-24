cmake_minimum_required(VERSION 3.16)
project(ais_lab4 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    example/main.cpp
    src/engine/Kohonen.cpp
    src/engine/SvgPlot.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE include)
