# ivf2 examples
#
# All examples share one runtime directory (${IVF2_RUNTIME_OUTPUT_DIR}); the
# shared runtime data and example assets are copied there once per build.

if(CMAKE_VERSION VERSION_LESS 3.26)
    set(_ivf2_copy copy_directory)
else()
    set(_ivf2_copy copy_directory_if_different)
endif()

# Web builds pack the data each page needs into its .data file instead (ivf2_web_executable).
if(NOT EMSCRIPTEN)
    add_custom_target(copy_example_assets ALL
        COMMAND ${CMAKE_COMMAND} -E ${_ivf2_copy} "${IVF2_RUNTIME_DATA_DIR}/shaders" "${IVF2_RUNTIME_OUTPUT_DIR}/shaders"
        COMMAND ${CMAKE_COMMAND} -E ${_ivf2_copy} "${IVF2_RUNTIME_DATA_DIR}/fonts"   "${IVF2_RUNTIME_OUTPUT_DIR}/fonts"
        COMMAND ${CMAKE_COMMAND} -E ${_ivf2_copy} "${CMAKE_CURRENT_SOURCE_DIR}/assets" "${IVF2_RUNTIME_OUTPUT_DIR}/assets"
        COMMENT "Copying shared runtime data and example assets to ${IVF2_RUNTIME_OUTPUT_DIR}"
        VERBATIM)
    set_target_properties(copy_example_assets PROPERTIES FOLDER "examples")
endif()

# Shared PCH holder: compiled once, reused by all examples that have no extra libs.
# Examples with EXTRA_LIBS (ivfvis, ivfphysics, ...) may get different include
# paths and cannot safely REUSE_FROM this target; they compile their own PCH.
add_library(ivf2_examples_pch STATIC pch_holder.cpp)
target_link_libraries(ivf2_examples_pch PRIVATE ivf2::ivf2)
ivf2_apply_compiler_settings(ivf2_examples_pch)
if(IVF2_USE_PCH)
    target_precompile_headers(ivf2_examples_pch PRIVATE pch.h)
endif()
set_target_properties(ivf2_examples_pch PROPERTIES FOLDER "examples")

# add_ivf2_example(<name> [SOURCES <files...>] [EXTRA_LIBS <targets...>] [WEB_ASSETS <paths...>])
#
# WEB_ASSETS: files or directories below assets/ that the web build (preset wasm) packs
# into the page's data file; shaders/ and fonts/ are always included. Desktop builds read
# everything from the copied assets/ directory and ignore it.
function(add_ivf2_example name)
    cmake_parse_arguments(ARG "" "" "SOURCES;EXTRA_LIBS;WEB_ASSETS" ${ARGN})

    if(NOT ARG_SOURCES)
        set(ARG_SOURCES ${name}.cpp)
    endif()

    add_executable(${name} ${ARG_SOURCES})
    target_link_libraries(${name} PRIVATE ivf2::ivf2 ${ARG_EXTRA_LIBS})
    ivf2_apply_compiler_settings(${name})
    set_target_properties(${name} PROPERTIES
        FOLDER "examples"
        DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${name}>")
    if(EMSCRIPTEN)
        ivf2_web_executable(${name} ASSETS_ROOT "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/assets" ASSETS ${ARG_WEB_ASSETS})
    else()
        add_dependencies(${name} copy_example_assets)
    endif()

    if(IVF2_USE_PCH)
        if(ARG_EXTRA_LIBS)
            # CMAKE_CURRENT_FUNCTION_LIST_DIR resolves to examples/ regardless of caller.
            target_precompile_headers(${name} PRIVATE "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/pch.h")
        else()
            target_precompile_headers(${name} REUSE_FROM ivf2_examples_pch)
        endif()
    endif()
endfunction()

# Web builds (preset wasm) skip the examples the browser cannot run: several windows or a
# host-owned GLFW loop, audio, and native file dialogs (ROADMAP phase 3.3).
set(IVF2_WEB_EXCLUDED_EXAMPLES multi_window1 embedding1 audio_reactive1 music_scene1 assimp1)

# Like add_subdirectory(), but skips the examples excluded from web builds.
function(ivf2_example_subdirectory name)
    if(EMSCRIPTEN AND name IN_LIST IVF2_WEB_EXCLUDED_EXAMPLES)
        return()
    endif()
    add_subdirectory(${name})
endfunction()

ivf2_example_subdirectory(hello_ivf)
ivf2_example_subdirectory(hello_pipeline)
ivf2_example_subdirectory(ui1)
ivf2_example_subdirectory(ui2)
ivf2_example_subdirectory(textures1)
ivf2_example_subdirectory(textures2)
ivf2_example_subdirectory(textures3)
ivf2_example_subdirectory(primitives1)
ivf2_example_subdirectory(lighting1)
ivf2_example_subdirectory(text1)
ivf2_example_subdirectory(extrusions1)
ivf2_example_subdirectory(transforms1)
ivf2_example_subdirectory(animation1)
ivf2_example_subdirectory(animation2)
ivf2_example_subdirectory(scene_ui1)
ivf2_example_subdirectory(selection1)
ivf2_example_subdirectory(animation3)
if(IVF2_WITH_ASSIMP)
    ivf2_example_subdirectory(assimp1)
endif()
ivf2_example_subdirectory(effects1)
ivf2_example_subdirectory(shadows1)
ivf2_example_subdirectory(deformer1)
ivf2_example_subdirectory(deformer2)
ivf2_example_subdirectory(deformer3)
ivf2_example_subdirectory(editor1)
if(TARGET ivf2::ivfphysics)
    ivf2_example_subdirectory(rigid_body1)
    ivf2_example_subdirectory(physics_joints1)
endif()
ivf2_example_subdirectory(solar_system)
ivf2_example_subdirectory(bbox1)
ivf2_example_subdirectory(procedural1)
ivf2_example_subdirectory(procedural2)
ivf2_example_subdirectory(particles1)
ivf2_example_subdirectory(behaviors1)
ivf2_example_subdirectory(custom_mesh1)
ivf2_example_subdirectory(tweening1)
ivf2_example_subdirectory(debug_draw1)
ivf2_example_subdirectory(skybox1)
ivf2_example_subdirectory(textures4)
ivf2_example_subdirectory(audio_reactive1)
ivf2_example_subdirectory(serialization1)
ivf2_example_subdirectory(bump1)
ivf2_example_subdirectory(time_control1)
ivf2_example_subdirectory(camera_anim1)
ivf2_example_subdirectory(flow_field1)
ivf2_example_subdirectory(timeline1)
ivf2_example_subdirectory(shader_material1)
ivf2_example_subdirectory(fog1)
ivf2_example_subdirectory(mip_bloom1)
ivf2_example_subdirectory(transparency1)
if(IVF2_BUILD_MUSIC)
    ivf2_example_subdirectory(music_scene1)
endif()
ivf2_example_subdirectory(instancing1)
ivf2_example_subdirectory(auto_instancing1)
ivf2_example_subdirectory(hdr1)
ivf2_example_subdirectory(ibl1)
ivf2_example_subdirectory(cascaded_shadows1)
ivf2_example_subdirectory(point_shadows1)
ivf2_example_subdirectory(ssao1)
ivf2_example_subdirectory(ssr1)
ivf2_example_subdirectory(gltf1)
ivf2_example_subdirectory(hot_reload1)
ivf2_example_subdirectory(picking1)
if(TARGET ivf2::ivfvis)
    ivf2_example_subdirectory(vis_axes1)
    ivf2_example_subdirectory(vis_colormaps1)
    ivf2_example_subdirectory(vis_dashboard1)
    ivf2_example_subdirectory(vis_points1)
    if(IVF2_WITH_VTK_XML)
        ivf2_example_subdirectory(vis_readers1)
    endif()
    ivf2_example_subdirectory(vis_scalar1)
    ivf2_example_subdirectory(vis_vectors1)
    ivf2_example_subdirectory(vis_volume1)
endif()
ivf2_example_subdirectory(dof1)
ivf2_example_subdirectory(reflection1)
ivf2_example_subdirectory(frame_export1)
ivf2_example_subdirectory(multi_window1)
ivf2_example_subdirectory(embedding1)
