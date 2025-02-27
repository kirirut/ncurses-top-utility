cmake_minimum_required(VERSION 3.10)


project("ncurses top utility")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.c
    src/process.c
)

set(HEADERS
    include/process.h
)

add_executable("ncurses_top_utility" ${SOURCES} ${HEADERS})

target_include_directories("ncurses_top_utility" PRIVATE include)

target_link_libraries("ncurses_top_utility" PRIVATE ncurses)