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
