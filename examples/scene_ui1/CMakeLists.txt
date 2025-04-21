file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(scene_ui1d scene_ui1.cpp example_window_1.h example_window_2.h example_window_1.cpp example_window_2.cpp)
    target_link_libraries(scene_ui1d PRIVATE ivfuid ivfd generatord gladd glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype)
    target_include_directories(scene_ui1d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(scene_ui1d gladd generatord ivfd ivfuid)
    set_target_properties(scene_ui1d PROPERTIES FOLDER "examples")
else()
    add_executable(scene_ui1 scene_ui1.cpp example_window_1.h example_window_2.h example_window_1.cpp example_window_2.cpp)
    target_link_libraries(scene_ui1 PRIVATE ivfui ivf  generator glad glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype)
    target_include_directories(scene_ui1 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(scene_ui1 glad generator ivf ivfui)
    set_target_properties(scene_ui1 PROPERTIES FOLDER "examples")
endif()


