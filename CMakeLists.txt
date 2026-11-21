cmake_minimum_required(VERSION 3.24)

# ---------------------------------------------------------------------------
# Pre-project setup (only when ivf2 is the top-level project)
# ---------------------------------------------------------------------------

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(_ivf2_top_level ON)
else()
    set(_ivf2_top_level OFF)
endif()

option(IVF2_BUILD_EXAMPLES "Build the ivf2 example programs" ${_ivf2_top_level})
option(IVF2_BUILD_TESTS "Build the ivf2 unit tests (CTest)" ${_ivf2_top_level})
option(IVF2_BUILD_MUSIC "Build the ivfmusic library (music-driven scenes)" ON)
option(IVF_WITH_RENOISE "Read Renoise songs (.xrns) in ivfmusic (requires miniz and pugixml)" ON)
option(IVF2_INSTALL "Generate install rules and a CMake package (find_package(ivf2))" ${_ivf2_top_level})
option(IVF2_USE_PCH "Use precompiled headers" ON)
option(IVF2_WARNINGS "Enable extra compiler warnings for ivf2 targets" OFF)
option(IVF2_OUTPUT_IN_SOURCE_TREE "Place binaries in <source>/bin/<config> and <source>/lib/<config> (legacy layout)" OFF)

if(_ivf2_top_level)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/IvfVcpkgBootstrap.cmake)

    # Share installed vcpkg packages across all build directories.
    set(VCPKG_INSTALLED_DIR "${CMAKE_CURRENT_LIST_DIR}/vcpkg_installed" CACHE PATH "vcpkg installed directory")

    # Optional vcpkg manifest features follow the CMake options.
    if(IVF2_BUILD_EXAMPLES)
        list(APPEND VCPKG_MANIFEST_FEATURES "examples")
    endif()
    if(IVF2_BUILD_MUSIC AND IVF_WITH_RENOISE)
        list(APPEND VCPKG_MANIFEST_FEATURES "renoise")
    endif()

    include(${CMAKE_CURRENT_LIST_DIR}/cmake/IvfAppleSdk.cmake)
endif()

# ---------------------------------------------------------------------------
# Project
# ---------------------------------------------------------------------------

project(ivf2
    VERSION 2.0.0
    DESCRIPTION "Interactive Visualisation Framework 2 - a C++20 scene graph library for OpenGL"
    HOMEPAGE_URL "https://github.com/jonaslindemann/ivf2"
    LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

if(_ivf2_top_level)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    get_property(_ivf2_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(NOT _ivf2_multi_config AND NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
    endif()

    if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
        set(CMAKE_DEBUG_POSTFIX d)
    endif()
endif()

# Output layout: <root>/bin/<Config> for executables and runtime data,
# <root>/lib/<Config> for libraries. Using $<CONFIG> keeps single- and
# multi-config generators identical.
if(IVF2_OUTPUT_IN_SOURCE_TREE)
    set(IVF2_OUTPUT_ROOT "${PROJECT_SOURCE_DIR}")
else()
    set(IVF2_OUTPUT_ROOT "${PROJECT_BINARY_DIR}")
endif()
set(IVF2_RUNTIME_OUTPUT_DIR "${IVF2_OUTPUT_ROOT}/bin/$<CONFIG>")
if(_ivf2_top_level)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${IVF2_RUNTIME_OUTPUT_DIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${IVF2_OUTPUT_ROOT}/lib/$<CONFIG>")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${IVF2_OUTPUT_ROOT}/lib/$<CONFIG>")
endif()

# Runtime data (shaders, fonts) shipped with the library, and example assets.
set(IVF2_RUNTIME_DATA_DIR "${PROJECT_SOURCE_DIR}/bin" CACHE INTERNAL "ivf2 runtime data (shaders/, fonts/)")

include(IvfTargets)
include(ivf2Helpers)
include(ivfmusic)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(RtMidi CONFIG REQUIRED)

# Header-only single-file libraries without CMake targets.
find_package(Stb QUIET)
if(NOT Stb_INCLUDE_DIR)
    find_path(Stb_INCLUDE_DIR stb_image.h PATH_SUFFIXES stb REQUIRED)
endif()
find_path(IVF2_MINIAUDIO_INCLUDE_DIR miniaudio.h PATH_SUFFIXES miniaudio)

if(IVF2_BUILD_MUSIC AND IVF_WITH_RENOISE)
    find_package(miniz CONFIG REQUIRED)
    find_package(pugixml CONFIG REQUIRED)
endif()

# ---------------------------------------------------------------------------
# Libraries
# ---------------------------------------------------------------------------

add_subdirectory(src)

# Unified convenience target: everything except ivfmusic.
add_library(ivf2 INTERFACE)
add_library(ivf2::ivf2 ALIAS ivf2)
target_link_libraries(ivf2 INTERFACE ivfui ivf ivfmath generator glad)
set_property(GLOBAL APPEND PROPERTY IVF2_EXPORTED_TARGETS ivf2)

# ---------------------------------------------------------------------------
# Examples, tests, install
# ---------------------------------------------------------------------------

if(IVF2_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(IVF2_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(IVF2_INSTALL)
    include(IvfInstall)
endif()

if(_ivf2_top_level)
    message(STATUS "")
    message(STATUS "ivf2 ${PROJECT_VERSION}")
    message(STATUS "  Examples ............ ${IVF2_BUILD_EXAMPLES}")
    message(STATUS "  Tests ............... ${IVF2_BUILD_TESTS}")
    message(STATUS "  ivfmusic ............ ${IVF2_BUILD_MUSIC} (Renoise: ${IVF_WITH_RENOISE})")
    message(STATUS "  Install rules ....... ${IVF2_INSTALL}")
    message(STATUS "  Output root ......... ${IVF2_OUTPUT_ROOT}")
    message(STATUS "")
endif()
