file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_5d example_5.cpp)
    target_link_libraries(example_5d PRIVATE ivfd ivfuid generatord gladd glfw stbd imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL glm::glm)
    target_include_directories(example_5d PRIVATE ${Stb_INCLUDE_DIR})
else()
endif()


