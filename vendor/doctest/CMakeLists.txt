cmake_minimum_required(VERSION 3.10)
project(doctest)

add_library(doctest INTERFACE)
add_library(doctest::doctest ALIAS doctest)
target_sources(
    doctest
    INTERFACE
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS ./
    FILES
        doctest.h
)
target_precompile_headers(doctest INTERFACE <doctest.h>)
