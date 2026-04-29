cmake_minimum_required(VERSION 3.10)

project(JackCompiler C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

include_directories(include)

file(GLOB SOURCES "src/*.c")

add_executable(jack main.c ${SOURCES})

enable_testing()
