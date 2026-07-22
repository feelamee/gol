cmake_minimum_required(VERSION 3.10)
project(gol)

set(options)
macro(opt name type doc val)
    list(APPEND options ${name})
    set(${name} ${val} CACHE ${type} ${doc})
endmacro()

opt(GOL_DEV BOOL "build for development" OFF)
opt(GOL_SANITIZERS BOOL "enable sanitizers" ${GOL_DEV})
opt(GOL_ASSERTIONS BOOL "enable assertions" ${GOL_DEV})
opt(GOL_TESTS BOOL "build tests" ${GOL_DEV})
opt(GOL_CCACHE BOOL "use ccache" ${GOL_DEV})
opt(GOL_HARDENING BOOL "enable hardening" OFF)
opt(GOL_USE_LIBRARIES STRING "way to get some dependencies" "VENDORED") # accepts "VENDORED", "SYSTEM", "DOWNLOAD"


if (GOL_CCACHE)
    find_program(CCACHE ccache)
    if (CCACHE)
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
    else()
        set(GOL_CCACHE_REAL OFF)
    endif()
endif()


message(STATUS)
message(STATUS "GOL was configured with the following options:")
foreach(opt IN LISTS options)
    if (DEFINED ${opt}_REAL)
        message(STATUS "    ${opt}: ${${opt}_REAL} (Wanted: ${${opt}})")
    else()
        message(STATUS "    ${opt}: ${${opt}}")
    endif()
endforeach()

message(STATUS)

message(STATUS "    CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")

if (DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "    CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
endif()

message(STATUS "    CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "    CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "    CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message(STATUS "    CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "    CMAKE_GENERATOR: ${CMAKE_GENERATOR}")

message(STATUS)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(common)
include(dependencies)


add_subdirectory(engine)
add_subdirectory(gol)
