# ivfmusic_add_executable(<name> SOURCES <files...> [ASSETS <dir>] [LIBS <targets...>])
#
# Builds an application on ivfmusic (GLFWMusicSceneWindow). When ASSETS is given:
#  - the directory is copied next to the executable as assets/ after each build
#    (only changed files), and
#  - IVFMUSIC_ASSET_SOURCE_DIR is defined, so ivfmusic::run() makes
#    MusicSceneConfig::resolve() read "assets/..." paths straight from the
#    project directory. Edits to automation files and shaders are then picked up
#    without a rebuild.
#
# LIBS selects the ivfmusic target to link (default: ivf2::ivfmusic), for
# projects that import prebuilt ivf2 libraries under other target names.

function(ivfmusic_add_executable name)
    cmake_parse_arguments(ARG "" "ASSETS" "SOURCES;LIBS" ${ARGN})

    if(NOT ARG_LIBS)
        set(ARG_LIBS ivf2::ivfmusic)
    endif()

    add_executable(${name} ${ARG_SOURCES})
    target_link_libraries(${name} PRIVATE ${ARG_LIBS})
    set_target_properties(${name} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${name}>"
    )
    if(MSVC)
        target_compile_options(${name} PRIVATE /utf-8)
    endif()

    if(ARG_ASSETS)
        get_filename_component(_assets "${ARG_ASSETS}" ABSOLUTE)
        target_compile_definitions(${name} PRIVATE IVFMUSIC_ASSET_SOURCE_DIR="${_assets}")
        if(CMAKE_VERSION VERSION_LESS 3.26)
            set(_copy copy_directory)
        else()
            set(_copy copy_directory_if_different)
        endif()
        add_custom_command(TARGET ${name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E ${_copy} "${_assets}" "$<TARGET_FILE_DIR:${name}>/assets"
            COMMENT "Copying assets of ${name}"
        )
    endif()
endfunction()
