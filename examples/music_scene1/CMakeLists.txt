ivfmusic_add_executable(music_scene1
    SOURCES music_scene1.cpp
    ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/assets
)
ivf2_apply_compiler_settings(music_scene1)
set_target_properties(music_scene1 PROPERTIES FOLDER "examples")
add_dependencies(music_scene1 copy_example_assets)
