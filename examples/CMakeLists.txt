# Create a custom target that copies all shared assets once to Debug and Release directories
# This is more efficient than copying for each example individually
add_custom_target(copy_example_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/bin/shaders ${PROJECT_SOURCE_DIR}/bin/Debug/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/bin/fonts ${PROJECT_SOURCE_DIR}/bin/Debug/fonts
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/bin/assets ${PROJECT_SOURCE_DIR}/bin/Debug/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/bin/shaders ${PROJECT_SOURCE_DIR}/bin/Release/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/bin/fonts ${PROJECT_SOURCE_DIR}/bin/Release/fonts
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/bin/assets ${PROJECT_SOURCE_DIR}/bin/Release/assets
    COMMENT "Copying shared assets (shaders, fonts, assets) to bin/Debug and bin/Release directories"
)

# Shared PCH holder: compiled once, reused by all examples that have no extra libs.
# Examples with EXTRA_LIBS (assimp, ReactPhysics3D, …) get different include paths
# and cannot safely REUSE_FROM this target; they compile their own PCH instead.
add_library(ivf2_examples_pch STATIC pch_holder.cpp)
target_link_libraries(ivf2_examples_pch PRIVATE ivf2::ivf2)
target_precompile_headers(ivf2_examples_pch PRIVATE pch.h)
set_target_properties(ivf2_examples_pch PROPERTIES
    FOLDER "examples"
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
)

# Helper function to create ivf2 examples with simplified dependency management
function(add_ivf2_example name)
    cmake_parse_arguments(ARG "" "" "SOURCES;EXTRA_LIBS" ${ARGN})

    if(NOT ARG_SOURCES)
        set(ARG_SOURCES ${name}.cpp)
    endif()

    add_executable(${name} ${ARG_SOURCES})
    target_link_libraries(${name} PRIVATE ivf2::ivf2 ${ARG_EXTRA_LIBS})
    set_target_properties(${name} PROPERTIES
        FOLDER "examples"
        DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    )
    add_dependencies(${name} glad generator ivf ivfui copy_example_assets)

    if(ARG_EXTRA_LIBS)
        # Extra libs may add include paths that differ from the shared PCH holder,
        # so compile a private PCH for this target.
        # CMAKE_CURRENT_FUNCTION_LIST_DIR always resolves to the directory of this
        # CMakeLists.txt (examples/), regardless of which subdirectory called the function.
        target_precompile_headers(${name} PRIVATE
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/pch.h"
        )
    else()
        # Standard examples: reuse the one PCH compiled by ivf2_examples_pch.
        target_precompile_headers(${name} REUSE_FROM ivf2_examples_pch)
    endif()
endfunction()

add_subdirectory(hello_ivf)
add_subdirectory(hello_pipeline)
add_subdirectory(ui1)
add_subdirectory(ui2)
add_subdirectory(textures1)
add_subdirectory(textures2)
add_subdirectory(textures3)
add_subdirectory(primitives1)
add_subdirectory(lighting1)
add_subdirectory(text1)
add_subdirectory(extrusions1)
add_subdirectory(transforms1)
add_subdirectory(animation1)
add_subdirectory(animation2)
add_subdirectory(scene_ui1)
add_subdirectory(selection1)
add_subdirectory(animation3)
add_subdirectory(assimp1)
add_subdirectory(effects1)
add_subdirectory(shadows1)
add_subdirectory(deformer1)
add_subdirectory(deformer2)
add_subdirectory(deformer3)
add_subdirectory(editor1)
add_subdirectory(rigid_body1)
add_subdirectory(solar_system)
add_subdirectory(bbox1)
add_subdirectory(procedural1)
add_subdirectory(procedural2)
add_subdirectory(particles1)
add_subdirectory(behaviors1)
add_subdirectory(custom_mesh1)
add_subdirectory(tweening1)
add_subdirectory(debug_draw1)
add_subdirectory(skybox1)
add_subdirectory(textures4)
add_subdirectory(audio_reactive1)
add_subdirectory(serialization1)