cmake_minimum_required(VERSION 3.5.1)
project(citadel-automatch-source)

# Dependency versions
set(RAPIDJSON_VERSION "v1.1.0")
set(SOURCE_SDK_VERSION "sdk2013")
set(METAMOD_VERSION "mmsource-1.10.6")
set(CATCH_VERSION "v2.1.1")

# Set the build directory
set(BUILD_DIR ${CMAKE_BINARY_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})

# Put dependencies in /vendor
set(EXT_PROJECTS_DIR ${CMAKE_BINARY_DIR}/vendor)

# Some global defs
add_definitions(-DRAD_TELEMETRY_DISABLED -DVERSION_SAFE_STEAM_API_INTERFACES)

# 2 sub-projects
add_subdirectory(${CMAKE_BINARY_DIR}/src)
add_subdirectory(${CMAKE_BINARY_DIR}/test)

# Testing
enable_testing()
add_test(NAME unittests COMMAND ${BUILD_DIR}/tests) # Test using the tests project

set(SOURCE_LIB_DIR ${SOURCE_SDK_DIR}/lib/public/linux32)
set_property(TEST unittests PROPERTY ENVIRONMENT "LD_LIBRARY_PATH=${SOURCE_LIB_DIR}")
