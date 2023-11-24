file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_3d example_3.cpp)
    target_link_libraries(example_3d PRIVATE ivfd ivfuid generatord gladd glfw stbd imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL glm::glm)
    target_include_directories(example_3d PRIVATE ${Stb_INCLUDE_DIR})
else()
endif()


