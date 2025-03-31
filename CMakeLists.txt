cmake_minimum_required(VERSION 3.10)
project(aleto)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(libs/wxWidgets-3.2.7)

add_executable(main src/main.cpp)
target_link_libraries(main wx::net wx::core wx::base)
