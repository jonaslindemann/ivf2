file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_5d example_5.cpp)
    target_link_libraries(example_5d PRIVATE gladd ivfd ivfuid generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype)
    target_include_directories(example_5d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_5d gladd generatord ivfd ivfuid)
else()
    add_executable(example_5 example_5.cpp)
    target_link_libraries(example_5 PRIVATE ivf ivfui generator glad glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype)
    target_include_directories(example_5 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_5 glad generator ivf ivfui)
endif()


