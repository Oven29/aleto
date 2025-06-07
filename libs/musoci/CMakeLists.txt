cmake_minimum_required(VERSION 3.23)

set(project "musoci")
project(${project})

set(CMAKE_CXX_STANDARD 17)

set(${project}_SOURCES
    sqlite.cpp
    postgresql.cpp
)

set(${project}_HEADERS
    types.hpp
    base.hpp
    sqlite.hpp
    postgresql.hpp
)

set(${project}_SOURCE_LIST
    ${${project}_SOURCES}
    ${${project}_HEADERS}
)

add_library(${project}
    STATIC
    ${${project}_SOURCE_LIST}
)

target_sources(musoci PRIVATE ../sqlite3/sqlite3.c)

set_target_properties(${project} PROPERTIES LINKER_LANGUAGE CXX)
