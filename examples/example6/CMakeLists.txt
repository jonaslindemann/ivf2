file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_6d example_6.cpp text_window.h text_window.cpp)
    target_link_libraries(example_6d PRIVATE ivfd ivfuid generatord gladd glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(example_6d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_6d gladd generatord ivfd ivfuid)
else()
    add_executable(example_6 example_6.cpp text_window.h text_window.cpp)
    target_link_libraries(example_6 PRIVATE ivf ivfui generator glad glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(example_6 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_6 glad generator ivf ivfui)
endif()


