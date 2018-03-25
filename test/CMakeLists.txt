# Make CMake automagically download Catch.
include(ExternalProject)
find_package(Git REQUIRED)

ExternalProject_Add(
    catch
    PREFIX ${EXT_PROJECTS_DIR}/catch
    GIT_REPOSITORY https://github.com/philsquared/Catch.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} fetch && ${GIT_EXECUTABLE} checkout ${CATCH_VERSION}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)

# Expose required variable (CATCH_INCLUDE_DIR) to parent scope
ExternalProject_Get_Property(catch source_dir)
set(CATCH_INCLUDE_DIR ${source_dir}/single_include CACHE INTERNAL "Path to include folder for Catch")

# Prepare "Catch" library for other executables
add_library(libcatch INTERFACE)
target_include_directories(libcatch INTERFACE ${CATCH_INCLUDE_DIR})

# Include this project's header files
include_directories(${CMAKE_BINARY_DIR}/include ${CMAKE_BINARY_DIR}/test)

# Make test executable
file(GLOB_RECURSE TEST_SOURCES "*.cpp")
add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests libcatch citadel-automatch dl)

# TODO: Move these to the top-level config. Avoids duplication in src project.
# Set compile flags
set_target_properties(tests PROPERTIES
    COMPILE_FLAGS "-march=pentium4 -m32 -std=c++1z"
    LINK_FLAGS "-lrt -m32 -mmmx -msse -Wl,--no-gnu-unique -fuse-ld=gold -static-libstdc++ -static-libgcc"
)
