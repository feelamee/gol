cmake_minimum_required(VERSION 3.10)
project(gol::engine LANGUAGES CXX)

# cmake omit -std=c++20 flag when using cxx_std_20 (probably because it is default in GCC 16)
# and clangd complain without it for some reason...
set(CMAKE_CXX_STANDARD 20)

add_library(engine)
add_library(engine::engine ALIAS engine)
target_sources(
    engine
    INTERFACE
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS include/
    FILES
		include/engine/fundamental.hpp
		include/engine/util.hpp
		include/engine/context.hpp
		include/engine/error.hpp
		include/engine/sdl.hpp
		include/engine/sdl_dialog.hpp
		include/engine/math.hpp
		include/engine/ranges.hpp
		include/engine/mesh.hpp
		include/engine/image.hpp
		include/engine/camera.hpp
		include/engine/gl.hpp
		include/engine/algorithm.hpp
		include/engine/imgui.hpp
		include/engine/log.hpp
		include/engine/scene.hpp

    PRIVATE
    FILE_SET private_headers
    TYPE HEADERS
    BASE_DIRS
        include/
        src/
    FILES
)
target_sources(
    engine
    PRIVATE
        src/util.cpp
        src/context.cpp
        src/sdl.cpp
        src/mesh.cpp
        src/image.cpp
        src/camera.cpp
        src/gl.cpp
        src/imgui.cpp
        src/log.cpp
        src/scene.cpp
)
target_compile_features(engine PUBLIC cxx_std_20)

target_link_libraries(
    engine
    PUBLIC
        $<$<BOOL:${GOL_DEV}>:dev_flags>
        $<$<BOOL:${GOL_SANITIZERS}>:sanitizers>
        $<$<BOOL:${GOL_ASSERTIONS}>:stdlib_assertions>
        $<$<BOOL:${GOL_HARDENING}>:hardening>
)

# TODO! make PRIVATE after removing all SDL/OpenGL code from public interface
target_link_libraries(engine PUBLIC SDL3::SDL3 glad::glad)

target_link_libraries(engine PUBLIC glm::glm imgui::imgui)

target_precompile_headers(
    engine
    PUBLIC
        <algorithm>
        <cassert>
        <charconv>
        <concepts>
        <cstddef>
        <cstdint>
        <filesystem>
        <memory>
        <ranges>
        <span>
        <stdexcept>
        <string>
        <string_view>
        <type_traits>
        <utility>
        <vector>
        <unordered_map>
        <map>
        <unordered_set>
        <set>
        <format>

        <SDL3/SDL_events.h>
        <SDL3/SDL_filesystem.h>
        <SDL3/SDL_gpu.h>
        <SDL3/SDL_hints.h>
        <SDL3/SDL_init.h>
        <SDL3/SDL_iostream.h>
        <SDL3/SDL_vulkan.h>

        <engine/fundamental.hpp>
        <engine/util.hpp>
        <engine/math.hpp>
        <engine/ranges.hpp>
)
