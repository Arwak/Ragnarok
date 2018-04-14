cmake_minimum_required(VERSION 3.8)
project(Ragnarok)

set(CMAKE_C_STANDARD 90)

set(SOURCE_FILES main.c filesystem/ext.c filesystem/filesystem.c)
add_executable(Ragnarok ${SOURCE_FILES})