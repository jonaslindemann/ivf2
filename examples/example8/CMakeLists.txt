file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_8d example_8.cpp)
    target_link_libraries(example_8d PRIVATE gladd ivfd ivfuid generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL glm::glm Freetype::Freetype) 
    target_include_directories(example_8d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_8d gladd generatord ivfd ivfuid)
else()
    add_executable(example_8 example_8.cpp)
    target_link_libraries(example_8 PRIVATE glad ivf ivfui generator glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL glm::glm Freetype::Freetype) 
    target_include_directories(example_8 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_8 glad generator ivf ivfui)
endif()


