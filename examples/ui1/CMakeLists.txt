file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(ui1d ui1.cpp example_window_1.h example_window_2.h example_window_1.cpp example_window_2.cpp)
    target_link_libraries(ui1d PRIVATE ivfuid ivfd ivfmathd generatord glfw gladd imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL)
    target_include_directories(ui1d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(ui1d gladd generatord ivfd ivfuid)
    set_target_properties(ui1d PROPERTIES FOLDER "examples")
else()
    add_executable(ui1 ui1.cpp example_window_1.h example_window_2.h example_window_1.cpp example_window_2.cpp)
    target_link_libraries(ui1 PRIVATE ivfui ivf ivfmath generator glfw glad imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL)
    target_include_directories(ui1 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(ui1 glad generator ivf ivfui)
    set_target_properties(ui1 PROPERTIES FOLDER "examples") 
endif()


