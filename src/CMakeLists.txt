# Make CMake automagically download metamod.
include(ExternalProject)
find_package(Git REQUIRED)

# Handle Metamod Dependency
ExternalProject_Add(
    metamod
    PREFIX ${EXT_PROJECTS_DIR}/metamod-source
    GIT_TAG ${METAMOD_VERSION}
    GIT_REPOSITORY https://github.com/alliedmodders/metamod-source.git
    GIT_SHALLOW 1
    GIT_PROGRESS 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TIMEOUT 10
    LOG_DOWNLOAD ON
)

# Expose required variable (METAMOD_INCLUDE_DIR) to parent scope
ExternalProject_Get_Property(metamod source_dir)
set(METAMOD_DIR ${source_dir} CACHE INTERNAL "Path to include folder for Metamod")
set(METAMOD_CORE_DIR ${METAMOD_DIR}/core)

# Metamod sources need to be compiled.
# Need an explicit list here due to metamod being metamod.
set(METAMOD_SOURCES
    "${METAMOD_CORE_DIR}/metamod.cpp"
    "${METAMOD_CORE_DIR}/metamod_util.cpp"
    "${METAMOD_CORE_DIR}/metamod_console.cpp"
    "${METAMOD_CORE_DIR}/metamod_oslink.cpp"
    "${METAMOD_CORE_DIR}/metamod_plugins.cpp"
    "${METAMOD_CORE_DIR}/sourcehook/sourcehook.cpp"
    "${METAMOD_CORE_DIR}/sourcehook/sourcehook_hookmangen.cpp"
    "${METAMOD_CORE_DIR}/sourcehook/sourcehook_impl_chookidman.cpp"
    "${METAMOD_CORE_DIR}/sourcehook/sourcehook_impl_chookmaninfo.cpp"
    "${METAMOD_CORE_DIR}/sourcehook/sourcehook_impl_cproto.cpp"
    "${METAMOD_CORE_DIR}/sourcehook/sourcehook_impl_cvfnptr.cpp"
    "${METAMOD_CORE_DIR}/provider/console.cpp"
    "${METAMOD_CORE_DIR}/provider/provider_ep2.cpp"
    "${METAMOD_CORE_DIR}/vsp_bridge.cpp"
    "${METAMOD_CORE_DIR}/gamedll_bridge.cpp"
)

set_source_files_properties(${METAMOD_SOURCES} PROPERTIES GENERATED TRUE)

# Handle Source SDK dependency
ExternalProject_Add(
    source-sdk
    PREFIX ${EXT_PROJECTS_DIR}/source-sdk
    GIT_TAG ${SOURCE_SDK_VERSION}
    # Need the alliedmodders mirror here, because valve
    GIT_REPOSITORY https://github.com/alliedmodders/hl2sdk.git
    GIT_SHALLOW 1
    GIT_PROGRESS 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TIMEOUT 10
    LOG_DOWNLOAD ON
)

# Expose SOURCE_SDK_DIR
ExternalProject_Get_Property(source-sdk source_dir)
set(SOURCE_SDK_DIR ${source_dir} CACHE INTERNAL "Path to include folder for Source SDK")

# Build the citadel-automatch plugin
file(GLOB SOURCES "*.cpp")
add_library(citadel-automatch SHARED ${SOURCES})

# Set include paths
include_directories(
    # Metamod header files
    "${METAMOD_DIR}/loader"
    "${METAMOD_DIR}/versionlib"
    "${METAMOD_CORE_DIR}"
    "${METAMOD_CORE_DIR}/sourcehook"
    # This projects header files
    "${CMAKE_BINARY_DIR}/include"
    # Source SDK header files
    "${SOURCE_SDK_DIR}/public"
    "${SOURCE_SDK_DIR}/public/tier0"
    "${SOURCE_SDK_DIR}/public/tier1"
    "${SOURCE_SDK_DIR}/public/engine"
    "${SOURCE_SDK_DIR}/public/game/server"
    "${SOURCE_SDK_DIR}/game/shared"
    "${SOURCE_SDK_DIR}/game/server"
)

# Don't add `lib` out the front of the library name
set_target_properties(citadel-automatch PROPERTIES PREFIX "")

set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)

# Platform specific settings
if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=pentium4 -m32 -std=c++17 -fpermissive -fno-rtti -fPIC -fno-exceptions -fno-rtti -DSOURCE_ENGINE=3 -DSE_EPISODEONE=1 -DSE_DARKMESSIAH=2 -DSE_ORANGEBOX=3 -DSE_BLOODYGOODTIME=4 -DSE_EYE=5 \
    -DSE_CSS=6 -DSE_ORANGEBOXVALVE=7 -DSE_LEFT4DEAD=8 -DSE_LEFT4DEAD2=9 -DSE_ALIENSWARM=10 \
    -DSE_PORTAL2=11 -DSE_CSGO=12 -DPOSIX -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp \
    -Dstrnicmp=strncasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf -D_alloca=alloca \
    -Dstrcmpi=strcasecmp -DCOMPILER_GCC")
    set(SOURCE_LIB_DIR ${SOURCE_SDK_DIR}/lib/public/linux32)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lrt -m32 -mmmx -msse -Wl,--no-gnu-unique -fuse-ld=gold -static-libstdc++ -static-libgcc -L${SOURCE_LIB_DIR}")

    add_definitions(-DPOSIX -DGNUC -DNO_MALLOC_OVERRIDE)

    if(APPLE)
        message("Detected OSX")

        add_definitions(-DOSX)

        # TODO: Libraries
    else() # Linux
        message("Detected Linux")

        add_definitions(-DLINUX -D_LINUX)

        # Search for libraries in the sdk
        # set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L${SOURCE_LIB_DIR}")

        # Static Libraries
        add_library(libtier1 STATIC IMPORTED)
        set_target_properties(libtier1 PROPERTIES IMPORTED_LOCATION ${SOURCE_LIB_DIR}/tier1.a)
        add_dependencies(libtier1 source-sdk)

        add_library(libtier2 STATIC IMPORTED)
        set_target_properties(libtier2 PROPERTIES IMPORTED_LOCATION ${SOURCE_LIB_DIR}/tier2.a)
        add_dependencies(libtier2 source-sdk)

        add_library(libtier3 STATIC IMPORTED)
        set_target_properties(libtier3 PROPERTIES IMPORTED_LOCATION ${SOURCE_LIB_DIR}/tier3.a)
        add_dependencies(libtier3 source-sdk)

        add_library(libmathlib STATIC IMPORTED)
        set_target_properties(libmathlib PROPERTIES IMPORTED_LOCATION ${SOURCE_LIB_DIR}/mathlib.a)
        add_dependencies(libmathlib source-sdk)

        # Dynamic Libraries
        # add_library(libsteam_api SHARED IMPORTED)
        # set_target_properties(libsteam_api PROPERTIES IMPORTED_LOCATION steam_api.so)
        # add_dependencies(libsteam_api source-sdk)

        # add_library(libtier0 SHARED IMPORTED)
        # set_target_properties(libtier0 PROPERTIES IMPORTED_LOCATION tier0.so)
        # add_dependencies(libtier0 source-sdk)

        # add_library(libvstdlib SHARED IMPORTED)
        # set_target_properties(libvstdlib PROPERTIES IMPORTED_LOCATION vstdlib.so)
        # add_dependencies(libvstdlib source-sdk)

        # Set linking
        target_link_libraries(citadel-automatch
            libtier1
            libtier2
            libtier3
            libmathlib
            steam_api
            tier0
            vstdlib
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
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install/addons/metamod
    CACHE PATH "default install path" FORCE)

# Install the library directly in the install dir
install(TARGETS citadel-automatch LIBRARY DESTINATION .
        # Its retarded that we need to set these manually...
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

# Also include the vdf in the install
install(FILES ${CMAKE_BINARY_DIR}/data/citadel-automatch.vdf DESTINATION .)
