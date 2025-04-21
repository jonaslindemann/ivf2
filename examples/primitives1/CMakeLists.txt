file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(primitives1d primitives1.cpp)
    target_link_libraries(primitives1d PRIVATE gladd ivfd ivfuid generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype)
    target_include_directories(primitives1d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(primitives1d gladd generatord ivfd ivfuid)
    set_target_properties(primitives1d PROPERTIES FOLDER "examples")
else()
    add_executable(primitives1d primitives1.cpp)
    target_link_libraries(primitives1d PRIVATE ivf ivfui generator glad glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype)
    target_include_directories(primitives1d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(primitives1d glad generator ivf ivfui)
    set_target_properties(primitives1 PROPERTIES FOLDER "examples")
endif()


