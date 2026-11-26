# ivf2Helpers.cmake - functions for applications built on ivf2.
#
# Available both in the ivf2 build tree and after find_package(ivf2).
#
#   ivf2_copy_runtime_files(<target> [ASSETS <dir>...])
#
# Copies the runtime data ivf2 needs at run time (shaders/, fonts/) next to the
# executable after each build. Extra ASSETS directories are copied as well,
# keeping their directory name (e.g. ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/assets
# ends up as <exe dir>/assets). Also points the Visual Studio debugger at the
# executable directory so relative resource paths resolve.

function(ivf2_copy_runtime_files target)
    cmake_parse_arguments(ARG "" "" "ASSETS" ${ARGN})

    if(CMAKE_VERSION VERSION_LESS 3.26)
        set(_copy copy_directory)
    else()
        set(_copy copy_directory_if_different)
    endif()

    set(_dirs "")
    foreach(_sub shaders fonts)
        if(EXISTS "${IVF2_RUNTIME_DATA_DIR}/${_sub}")
            list(APPEND _dirs "${IVF2_RUNTIME_DATA_DIR}/${_sub}")
        endif()
    endforeach()
    foreach(_a IN LISTS ARG_ASSETS)
        get_filename_component(_abs "${_a}" ABSOLUTE)
        list(APPEND _dirs "${_abs}")
    endforeach()

    foreach(_d IN LISTS _dirs)
        get_filename_component(_name "${_d}" NAME)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E ${_copy} "${_d}" "$<TARGET_FILE_DIR:${target}>/${_name}"
            VERBATIM)
    endforeach()

    set_target_properties(${target} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${target}>")
endfunction()

#   ivf2_web_executable(<target> [ASSETS_ROOT <dir>] [ASSETS <path>...] [SHELL <file>])
#
# Web builds (Emscripten) only: builds <target> as a web page, <target>.html with .js,
# .wasm and .data files. The .data file is a virtual file system preloaded before main():
# ivf2's shaders/ and fonts/ plus the ASSETS, so the program reads them with relative
# paths as on the desktop. An ASSETS path relative to ASSETS_ROOT keeps its place below the
# root's name (ASSETS_ROOT .../assets, ASSETS textures/brick.png -> assets/textures/brick.png);
# an absolute directory appears under its own name. Load only what the page needs: the
# whole data file is downloaded before the program starts. SHELL replaces the page
# template (ivf2_web_shell.html: a full-window canvas with a loading indicator).

set(_IVF2_HELPERS_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(ivf2_web_executable target)
    if(NOT EMSCRIPTEN)
        return()
    endif()
    cmake_parse_arguments(ARG "" "ASSETS_ROOT;SHELL" "ASSETS" ${ARGN})

    if(NOT ARG_SHELL)
        set(ARG_SHELL "${_IVF2_HELPERS_DIR}/ivf2_web_shell.html")
    endif()
    if(NOT ARG_ASSETS_ROOT)
        set(ARG_ASSETS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    get_filename_component(_root "${ARG_ASSETS_ROOT}" ABSOLUTE)
    get_filename_component(_root_name "${_root}" NAME)

    set(_preload "")
    foreach(_sub shaders fonts)
        if(EXISTS "${IVF2_RUNTIME_DATA_DIR}/${_sub}")
            list(APPEND _preload "--preload-file=${IVF2_RUNTIME_DATA_DIR}/${_sub}@/${_sub}")
        endif()
    endforeach()
    foreach(_a IN LISTS ARG_ASSETS)
        if(IS_ABSOLUTE "${_a}")
            get_filename_component(_name "${_a}" NAME)
            list(APPEND _preload "--preload-file=${_a}@/${_name}")
        else()
            if(NOT EXISTS "${_root}/${_a}")
                message(FATAL_ERROR "ivf2_web_executable(${target}): asset ${_root}/${_a} not found")
            endif()
            list(APPEND _preload "--preload-file=${_root}/${_a}@/${_root_name}/${_a}")
        endif()
    endforeach()

    set_target_properties(${target} PROPERTIES SUFFIX ".html")
    target_link_options(${target} PRIVATE
        ${_preload}
        "--shell-file=${ARG_SHELL}"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s STACK_SIZE=2MB"
        --profiling-funcs) # function names in browser stack traces
    set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS "${ARG_SHELL}")
endfunction()
