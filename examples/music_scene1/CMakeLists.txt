ivfmusic_add_executable(music_scene1
    SOURCES music_scene1.cpp
    ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/assets
)
set_target_properties(music_scene1 PROPERTIES
    FOLDER "examples"
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
)
add_dependencies(music_scene1 copy_example_assets)
