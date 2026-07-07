cmake_minimum_required(VERSION 3.10)
project(gol)


option(GOL_DEV "build for development" OFF)
option(GOL_SANITIZERS "enable sanitizers" ${GOL_DEV})
option(GOL_ASSERTIONS "enable assertions" ${GOL_DEV})
option(GOL_HARDENING "enable hardening" OFF)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(common)


add_subdirectory(vendor)
add_subdirectory(engine)
add_subdirectory(gol)
