cmake_minimum_required(VERSION 3.28)
project(MacroClion C)

set(CMAKE_C_STANDARD 99)

add_executable(MacroClion main.c
        macro.c
        macro.h
        input.txt)
