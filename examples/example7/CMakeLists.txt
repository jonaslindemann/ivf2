file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_7d example_7.cpp)
    target_link_libraries(example_7d PRIVATE gladd ivfd ivfuid generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL glm::glm Freetype::Freetype) 
    target_include_directories(example_7d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_7d gladd generatord ivfd ivfuid)
else()
    add_executable(example_7 example_7.cpp)
    target_link_libraries(example_7 PRIVATE glad ivf ivfui generator glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL glm::glm Freetype::Freetype) 
    target_include_directories(example_7 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_7 glad generator ivf ivfui)
endif()


