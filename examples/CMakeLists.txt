# Helper function to create ivf2 examples with simplified dependency management
function(add_ivf2_example name)
    cmake_parse_arguments(ARG "" "" "SOURCES;EXTRA_LIBS" ${ARGN})
    
    if(NOT ARG_SOURCES)
        set(ARG_SOURCES ${name}.cpp)
    endif()
    
    add_executable(${name} ${ARG_SOURCES})
    target_link_libraries(${name} PRIVATE ivf2::ivf2 ${ARG_EXTRA_LIBS})
    set_target_properties(${name} PROPERTIES FOLDER "examples")
    add_dependencies(${name} glad generator ivf ivfui)
    
    # Automatically copy shared assets to the executable directory after build
    # This ensures shaders, fonts, and other assets are available at runtime
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/bin/shaders $<TARGET_FILE_DIR:${name}>/shaders
        COMMENT "Copying shaders to ${name} output directory"
    )
    
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/bin/fonts $<TARGET_FILE_DIR:${name}>/fonts
        COMMENT "Copying fonts to ${name} output directory"
    )
    
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/bin/assets $<TARGET_FILE_DIR:${name}>/assets
        COMMENT "Copying assets to ${name} output directory"
    )
endfunction()

add_subdirectory(hello_ivf)
add_subdirectory(hello_pipeline)
add_subdirectory(ui1)
add_subdirectory(ui2)
add_subdirectory(textures1)
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