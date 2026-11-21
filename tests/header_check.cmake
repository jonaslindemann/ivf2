# Header self-containment / dependency hygiene check (build-only).
#
# For every public header of a library, compile a translation unit that
# includes only that header, seeing only the library's *public* usage
# requirements. This fails the build if a header
#   - includes a third-party library that is a private dependency
#     (e.g. nlohmann/json, assimp, FreeType, ImGui in ivf core), or
#   - is not self-contained (relies on includes made elsewhere, e.g. the PCH).

function(ivf2_add_header_check lib)
    file(GLOB _headers CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/${lib}/*.h")
    set(_dir "${CMAKE_CURRENT_BINARY_DIR}/header_check/${lib}")
    set(_sources "")
    foreach(_h IN LISTS _headers)
        get_filename_component(_name "${_h}" NAME_WE)
        set(_src "${_dir}/${_name}.cpp")
        set(_content "#include <${lib}/${_name}.h>\n")
        if(EXISTS "${_src}")
            file(READ "${_src}" _old)
        else()
            set(_old "")
        endif()
        if(NOT _old STREQUAL _content)
            file(WRITE "${_src}" "${_content}")
        endif()
        list(APPEND _sources "${_src}")
    endforeach()

    add_library(header_check_${lib} OBJECT ${_sources})
    target_link_libraries(header_check_${lib} PRIVATE ivf2::${lib})
    ivf2_apply_compiler_settings(header_check_${lib})
    set_target_properties(header_check_${lib} PROPERTIES FOLDER "tests" DISABLE_PRECOMPILE_HEADERS ON)
endfunction()
