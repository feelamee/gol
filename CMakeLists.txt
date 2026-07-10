cmake_minimum_required(VERSION 3.10)
project(gol)

set(options)
macro(opt name doc val)
    list(APPEND options ${name})
    option(${name} "${doc}" ${val})
endmacro()

opt(GOL_DEV "build for development" OFF)
opt(GOL_SANITIZERS "enable sanitizers" ${GOL_DEV})
opt(GOL_ASSERTIONS "enable assertions" ${GOL_DEV})
opt(GOL_TESTS "build tests" ${GOL_DEV})

opt(GOL_HARDENING "enable hardening" OFF)
# by default will download and build dependencies from source
opt(GOL_USE_SYSTEM_LIBRARIES "satisfy dependencies from system installed libraries" OFF)

message(STATUS)
message(STATUS "GOL was configured with the following options:")
foreach(opt IN LISTS options)
    message(STATUS "    ${opt}: ${${opt}}")
endforeach()
message(STATUS)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(common)
include(dependencies)


add_subdirectory(vendor)
add_subdirectory(engine)
add_subdirectory(gol)
