cmake_minimum_required(VERSION 4.2)
project(CComp C)

set(CMAKE_C_STANDARD 11)

add_executable(CComp main.c
        include/common.h
        include/tokenizer.h
        include/symtab.h
        include/vmwriter.h
        include/compile.h
        src/tokenizer.c
        src/symtab.c)
