cmake_minimum_required(VERSION 3.10)
project(nlohmann_json)

add_library(nlohmann_json INTERFACE)
add_library(nlohmann::json ALIAS nlohmann_json)
target_sources(
    nlohmann_json
    INTERFACE
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS ./
    FILES
        nlohmann_json.h
)
target_precompile_headers(nlohmann_json INTERFACE <nlohmann_json.hpp>)
