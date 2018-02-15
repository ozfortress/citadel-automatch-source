# Build the citadel-automatch plugin
file(GLOB SOURCES "*.cpp")
add_library(citadel-automatch SHARED ${SOURCES})

# Don't add `lib` out the front of the library name
set_target_properties(citadel-automatch PROPERTIES PREFIX "")

# Include directories from the source SDK
if(DEFINED SOURCE_SDK_DIR)
    message("Using source SDK found in ${SOURCE_SDK_DIR}")
else()
    message(SEND_ERROR "SOURCE_SDK_DIR is not defined. You must tell CMake where to find the source SDK")
endif()

# Fix the source path if its relative
if(NOT IS_ABSOLUTE SOURCE_SDK_DIR)
    set(SOURCE_SDK_DIR ${CMAKE_BINARY_DIR}/${SOURCE_SDK_DIR})
endif()

include_directories(
    # This projects header files
    "${CMAKE_BINARY_DIR}/include"
    # Source SDK header files
    "${SOURCE_SDK_DIR}/common"
    "${SOURCE_SDK_DIR}/public"
    "${SOURCE_SDK_DIR}/public/tier0"
    "${SOURCE_SDK_DIR}/public/tier1"
    "${SOURCE_SDK_DIR}/game/shared"
    "${SOURCE_SDK_DIR}/game/server"
)

set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)

# Platform specific settings
if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=pentium4 -m32 -std=c++11")
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
        set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${SOURCE_SDK_DIR}/lib/public/linux32")

        # Static Libraries
        find_library(tier1_LIB NAMES tier1.a)
        find_library(tier2_LIB NAMES tier2.a)
        find_library(tier3_LIB NAMES tier3.a)
        find_library(mathlib_LIB NAMES mathlib.a)

        # Dynamic Libraries
        find_library(steam_api_LIB NAMES steam_api)
        find_library(tier0_LIB NAMES tier0)
        find_library(vstdlib_LIB NAMES vstdlib)

        # Generate a nicer error
        if(NOT tier1_LIB OR NOT tier2_LIB OR NOT tier3_LIB OR NOT mathlib_LIB
            OR NOT steam_api_LIB OR NOT tier0_LIB OR NOT vstdlib_LIB)
            message(SEND_ERROR "Could not find some required steam-sdk libraries. Make sure SOURCE_SDK_DIR points to the 'mp/src' directory of the full sdk.")
        endif()

        target_link_libraries(citadel-automatch
            ${tier0_LIB}
            ${vstdlib_LIB}
            ${tier1_LIB}
            ${tier2_LIB}
            ${tier3_LIB}
            ${mathlib_LIB}
            pthread
            curl
        )
    endif()

elseif(WIN32)
    message("Detected Windows")

    add_definitions(-DWIN32 -D_WIN32)

    link_directories("${SOURCE_SDK_DIR}/lib/public")

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

# Install Rules
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install/addons
    CACHE PATH "default install path" FORCE)

# Install the library directly in the install dir
install(TARGETS citadel-automatch LIBRARY DESTINATION .
        # Its retarded that we need to set these manually...
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

# Also include the vdf in the install
install(FILES ${CMAKE_BINARY_DIR}/data/citadel-automatch.vdf DESTINATION .)