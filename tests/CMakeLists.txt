# ivf2 unit tests - GL-free components only (generator, ivfmath).
# Run with: ctest --preset <preset>   or   ctest --test-dir <build dir> -C <config>

function(ivf2_add_test name)
    cmake_parse_arguments(ARG "" "" "LIBS" ${ARGN})
    add_executable(${name} ${name}.cpp test_main.h)
    target_link_libraries(${name} PRIVATE ${ARG_LIBS})
    ivf2_apply_compiler_settings(${name})
    set_target_properties(${name} PROPERTIES FOLDER "tests")
    add_test(NAME ${name} COMMAND ${name})
endfunction()

ivf2_add_test(test_generator LIBS ivf2::generator)
ivf2_add_test(test_ivfmath LIBS ivf2::ivfmath)
ivf2_add_test(test_render_context LIBS ivf2::ivf)
ivf2_add_test(test_logger LIBS ivf2::ivf)
ivf2_add_test(test_result LIBS ivf2::ivf)
ivf2_add_test(test_scene_without_gl LIBS ivf2::ivf)
ivf2_add_test(test_render_queue LIBS ivf2::ivf)
ivf2_add_test(test_environment LIBS ivf2::ivf)
ivf2_add_test(test_shadow_math LIBS ivf2::ivf)

# Public headers must compile on their own with only public dependencies.
include(${CMAKE_CURRENT_SOURCE_DIR}/header_check.cmake)
ivf2_add_header_check(ivfmath)
ivf2_add_header_check(ivf)
ivf2_add_header_check(ivfui)
if(TARGET ivf2::ivfmusic)
    ivf2_add_header_check(ivfmusic)
endif()

# Rendered examples compared with reference images (needs IVF2_BUILD_EXAMPLES and a GL driver)
if(IVF2_VISUAL_TESTS AND IVF2_BUILD_EXAMPLES)
    add_subdirectory(visual)
endif()
