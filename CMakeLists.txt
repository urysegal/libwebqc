cmake_minimum_required(VERSION 3.21)
project(libwebqc C)

set(CMAKE_C_STANDARD 23)

add_library(libwebqc SHARED libwebqc.c)
