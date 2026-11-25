# Visual regression tests: each listed example runs hidden with a fixed time step, its last
# frame (without the UI) is saved and compared with a reference image within a tolerance.
#
#   cmake --preset release -DIVF2_VISUAL_TESTS=ON
#   ctest --preset release -L visual
#
# The references in reference/ are rendered by CI on Linux with Mesa's software renderer
# (llvmpipe), so other drivers produce slightly different images. To check your own changes
# with another GPU, point IVF2_VISUAL_REFERENCE_DIR at a directory of your own, fill it
# once with IVF2_UPDATE_REFERENCES=1 and compare against it afterwards. Rendered images and
# diffs are written to <build>/visual/.

set(IVF2_VISUAL_REFERENCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/reference" CACHE PATH
    "Reference images for the visual regression tests")
set(IVF2_VISUAL_THRESHOLD 24 CACHE STRING
    "Visual tests: a pixel differs when a channel differs by more than this (0-255)")
set(IVF2_VISUAL_MAX_FRACTION 0.01 CACHE STRING
    "Visual tests: largest fraction of differing pixels that still matches")

add_executable(image_compare image_compare.cpp)
target_link_libraries(image_compare PRIVATE ivf2::ivf)
ivf2_apply_compiler_settings(image_compare)
set_target_properties(image_compare PROPERTIES FOLDER "tests")

# ivf2_add_visual_test(<example> [FRAMES n] [TIMESTEP seconds])
# FRAMES frames are drawn (default 30) and time advances TIMESTEP per frame (default 1/30 s).
function(ivf2_add_visual_test example)
    cmake_parse_arguments(ARG "" "FRAMES;TIMESTEP" "" ${ARGN})
    if(NOT TARGET ${example})
        return()
    endif()
    if(NOT ARG_FRAMES)
        set(ARG_FRAMES 30)
    endif()
    if(NOT ARG_TIMESTEP)
        set(ARG_TIMESTEP 0.0333333)
    endif()

    add_test(NAME visual_${example}
        COMMAND ${CMAKE_COMMAND}
            -DNAME=${example}
            -DEXE=$<TARGET_FILE:${example}>
            -DWORKDIR=$<TARGET_FILE_DIR:${example}>
            -DCOMPARE=$<TARGET_FILE:image_compare>
            -DREFERENCE_DIR=${IVF2_VISUAL_REFERENCE_DIR}
            -DOUTPUT_DIR=${CMAKE_BINARY_DIR}/visual
            -DFRAMES=${ARG_FRAMES}
            -DTIMESTEP=${ARG_TIMESTEP}
            -DTHRESHOLD=${IVF2_VISUAL_THRESHOLD}
            -DMAX_FRACTION=${IVF2_VISUAL_MAX_FRACTION}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/run_visual_test.cmake)
    set_tests_properties(visual_${example} PROPERTIES LABELS visual TIMEOUT 300)
endfunction()

# Every example except embedding1 (raw GLFW, ignores the IVF2_* variables) and music_scene1
# (follows the audio clock; its own --export mode renders deterministic frames).
set(_ivf2_visual_examples
    animation1 animation2 animation3 assimp1 audio_reactive1 auto_instancing1 bbox1 behaviors1
    bump1 camera_anim1 cursor1 cursor_optimization custom_mesh1 debug_draw1 deformer1 deformer2
    deformer3 dof1
    editor1 effects1 extrusions1 flow_field1 fog1 frame_export1 gltf1 hdr1 hello_ivf hello_pipeline hot_reload1
    cascaded_shadows1 ibl1 instancing1 lighting1 mip_bloom1 multi_window1 particles1 picking1 primitives1 procedural1
    point_shadows1 procedural2 reflection1 rigid_body1 scene_ui1 selection1 serialization1 shader_material1
    shadows1 skybox1 solar_system ssao1 ssr1 text1 textures1 textures2 textures3 textures4 time_control1
    timeline1 transforms1 transparency1 tweening1 ui1 ui2 vis_colormaps1 vis_points1
)
foreach(example IN LISTS _ivf2_visual_examples)
    ivf2_add_visual_test(${example})
endforeach()
