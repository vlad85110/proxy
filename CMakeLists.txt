cmake_minimum_required(VERSION 3.21)
project(proxy C)

set(CMAKE_C_STANDARD 99)

add_executable(proxy main.c proxy.c list.c logger.h logger.c)
