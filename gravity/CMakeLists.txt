cmake_minimum_required(VERSION 3.10)
project(gravity::gravity LANGUAGES CXX)

# cmake omit -std=c++20 flag when using cxx_std_20 (probably because it is default in GCC 16)
# and clangd complain without it for some reason...
set(CMAKE_CXX_STANDARD 20)

add_executable(
    gravity-simulation
    src/main.cpp
)
target_link_libraries(gravity-simulation PRIVATE engine::engine)
