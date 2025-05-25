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
)

add_subdirectory(libs/wxWidgets-3.2.8)
add_subdirectory(libs/musoci)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS})

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -std=c++17)
target_link_libraries(${CMAKE_PROJECT_NAME} wx::net wx::core wx::base musoci)

if(ENABLE_DEBUG)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -g)
endif()

add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/libs/bin/sqldb-driver
            ${CMAKE_BINARY_DIR}/bin/sqldb-driver
    COMMAND ${CMAKE_COMMAND} -E echo "Copying sqldb-driver..."
)

if (UNIX)
    add_custom_command(
        TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND chmod +x ${CMAKE_BINARY_DIR}/bin/sqldb-driver
    )
endif()
