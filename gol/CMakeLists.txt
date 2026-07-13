cmake_minimum_required(VERSION 3.10)
project(gol::gol LANGUAGES CXX)

# cmake omit -std=c++20 flag when using cxx_std_20 (probably because it is default in GCC 16)
# and clangd complain without it for some reason...
set(CMAKE_CXX_STANDARD 20)

set(targets gol-simulation)
if (GOL_TESTS)
    list(APPEND targets gol-simulation-test)
endif()

foreach(target IN LISTS targets)
    add_executable(${target})
    target_sources(
        ${target}

        PRIVATE
            src/world.cpp
            src/world_json.cpp
            src/world_scene_node.cpp

        PRIVATE
        FILE_SET private_headers
        TYPE HEADERS
        BASE_DIRS
            include/
        FILES
            include/gol/world.hpp
            include/gol/world_json.hpp
            include/gol/world_scene_node.hpp
    )
    target_link_libraries(${target} PRIVATE engine::engine)
    target_link_libraries(${target} PRIVATE doctest::doctest)
    target_link_libraries(${target} PRIVATE nlohmann::json)
endforeach()

target_sources(gol-simulation PRIVATE src/main.cpp)
target_compile_definitions(gol-simulation PRIVATE DOCTEST_CONFIG_DISABLE)

if (GOL_TESTS)
    target_sources(gol-simulation-test PRIVATE src/main.test.cpp)
    # TODO! kludge, required because main.test.cpp add define before including doctest
    set_source_files_properties(src/main.test.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()


install(
    TARGETS gol-simulation
    RUNTIME DESTINATION bin
)

# TODO! add option GOL_INSTALL_TESTS?
if (GOL_TESTS)
    install(
        TARGETS gol-simulation-test
        RUNTIME DESTINATION bin
    )
endif()
