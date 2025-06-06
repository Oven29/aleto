cmake_minimum_required(VERSION 3.10)

project("aleto")

set(CMAKE_CXX_STANDARD 11)

set(SOURCES
    src/main.cpp
)

set(HEADERS
    src/App.hpp
    src/ui/MainFrame.hpp
    src/ui/MainFrame.hpp
    src/core/config.hpp
)

add_subdirectory(libs/wxWidgets-3.2.8)
add_subdirectory(libs/sqlite3)
add_subdirectory(libs/musoci)
add_subdirectory(libs/json)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS})

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -std=c++17)
target_link_libraries(${CMAKE_PROJECT_NAME} wx::net wx::core wx::base sqlite3 musoci json)

if(ENABLE_DEBUG)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -g)
endif()
