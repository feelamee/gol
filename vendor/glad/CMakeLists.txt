cmake_minimum_required(VERSION 3.12)
project(glad)


add_library(glad STATIC)
add_library(glad::glad ALIAS glad)

target_sources(
    glad
    PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS include/
    FILES
        include/glad/glad.h
        include/KHR/khrplatform.h

    PRIVATE
        src/glad.c
)
set_target_properties(glad PROPERTIES POSITION_INDEPENDENT_CODE ON)
