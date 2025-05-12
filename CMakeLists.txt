cmake_minimum_required(VERSION 3.10)


project("ncurses top utility")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.c 
    src/process.c
    src/ui.c
)

include_directories(include)

add_executable(ncurses_top_utility ${SOURCES})

target_include_directories("ncurses_top_utility" PRIVATE include)

target_link_libraries("ncurses_top_utility" PRIVATE ncurses)