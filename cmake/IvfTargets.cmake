# IvfTargets.cmake - internal helpers used to define the ivf2 library targets.

include(GNUInstallDirs)

# Header install prefix. Headers are installed below include/ivf2/ so the
# bundled third-party headers (glm, gml, glad, KHR) never clash with system copies.
set(IVF2_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/ivf2")

# ivf2_add_library(<name> [FOLDER <folder>] [PCH <header>] SOURCES <files...>)
#
# Creates a static library <name> with the alias ivf2::<name>, the common
# include directories, C++20, position independent code and the shared
# compiler settings. The target is registered for export.
function(ivf2_add_library name)
    cmake_parse_arguments(ARG "" "FOLDER;PCH" "SOURCES" ${ARGN})

    add_library(${name} STATIC ${ARG_SOURCES})
    add_library(ivf2::${name} ALIAS ${name})

    target_include_directories(${name} PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${IVF2_GENERATED_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${IVF2_INSTALL_INCLUDEDIR}>)

    target_compile_features(${name} PUBLIC cxx_std_20)

    # utils.h checks _DEBUG; MSVC defines it for debug runtimes, other compilers do not.
    target_compile_definitions(${name} PUBLIC
        $<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:_DEBUG>)

    ivf2_apply_compiler_settings(${name})

    if(ARG_PCH AND IVF2_USE_PCH)
        target_precompile_headers(${name} PRIVATE ${ARG_PCH})
    endif()

    if(NOT ARG_FOLDER)
        set(ARG_FOLDER "ivf")
    endif()

    set_target_properties(${name} PROPERTIES
        FOLDER "${ARG_FOLDER}"
        POSITION_INDEPENDENT_CODE ON
        DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
        EXPORT_NAME ${name})

    set_property(GLOBAL APPEND PROPERTY IVF2_EXPORTED_TARGETS ${name})
endfunction()

# ivf2_apply_compiler_settings(<target>)
#
# Shared, non-propagating compiler settings for ivf2 libraries, examples and tests.
function(ivf2_apply_compiler_settings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /utf-8 /Zc:__cplusplus)
        if(CMAKE_GENERATOR MATCHES "Visual Studio")
            target_compile_options(${target} PRIVATE /MP)
        endif()
        target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
    endif()

    if(IVF2_WARNINGS)
        if(MSVC)
            target_compile_options(${target} PRIVATE /W4)
        else()
            target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        endif()
    endif()
endfunction()
