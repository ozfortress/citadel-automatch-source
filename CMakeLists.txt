cmake_minimum_required(VERSION 3.5.1)
project (citadel-source-automatch)

# Set the build directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)


# Include directories from the source SDK
IF(DEFINED ENV{SOURCE_SDK_DIR})
  message("Using source SDK found in $ENV{SOURCE_SDK_DIR}")
ELSE()
  message("SOURCE_SDK_DIR is not defined. You must tell CMake where to find the source SDK")
  return()
ENDIF()

include_directories(
    "$ENV{SOURCE_SDK_DIR}/common"
    "$ENV{SOURCE_SDK_DIR}/public"
    "$ENV{SOURCE_SDK_DIR}/public/tier0"
    "$ENV{SOURCE_SDK_DIR}/public/tier1"
    "$ENV{SOURCE_SDK_DIR}/game/shared"
    "$ENV{SOURCE_SDK_DIR}/game/server"
)

# Use `src/` for sources
file(GLOB SOURCES "src/*.cpp")

add_definitions(-DRAD_TELEMETRY_DISABLED -DVERSION_SAFE_STEAM_API_INTERFACES)

# Build the citadel-automatch plugin
add_library(citadel-automatch SHARED ${SOURCES})

# Platform specific settings
if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=pentium4 -m32")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lrt -m32 -mmmx -msse -Wl,--no-gnu-unique -fuse-ld=gold -static-libstdc++ -static-libgcc" )

    add_definitions(-DPOSIX -DGNUC -DNO_MALLOC_OVERRIDE)

    if(APPLE)
        message("Detected OSX")
        add_definitions(-DOSX)

        # TODO: Libraries
    else() # Linux
        message("Detected Linux")

        add_definitions(-DLINUX -D_LINUX)

        # Search for libraries in the sdk
        set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "$ENV{SOURCE_SDK_DIR}/lib/public/linux32")

        # Static Libraries
        find_library(tier1_LIB NAMES tier1.a)
        find_library(tier2_LIB NAMES tier2.a)
        find_library(tier3_LIB NAMES tier3.a)
        find_library(mathlib_LIB NAMES mathlib.a)

        # Dynamic Libraries
        find_library(steam_api_LIB NAMES steam_api)
        find_library(tier0_LIB NAMES tier0)
        find_library(vstdlib_LIB NAMES vstdlib)

        target_link_libraries(citadel-automatch
            ${tier0_LIB}
            ${vstdlib_LIB}
            ${tier1_LIB}
            ${tier2_LIB}
            ${tier3_LIB}
            ${mathlib_LIB})
    endif()

elseif(WIN32)
    message("Detected Windows")

    add_definitions(-DWIN32 -D_WIN32)

    link_directories("$ENV{SOURCE_SDK_DIR}/lib/public")

    # TODO: Libraries

    # RAD_TELEMETRY_DISABLED \
    #             WIN32 \
    #             # _WIN32 \
    #             _CRT_SECURE_NO_DEPRECATE \
    #             _CRT_NONSTDC_NO_DEPRECATE \
    #             _MBCS \
    #             _ALLOW_RUNTIME_LIBRARY_MISMATCH \
    #             _ALLOW_ITERATOR_DEBUG_LEVEL_MISMATCH \
endif()