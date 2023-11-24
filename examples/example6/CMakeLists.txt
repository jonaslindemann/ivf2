file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_6d example_6.cpp text_window.h text_window.cpp)
    target_link_libraries(example_6d PRIVATE ivfd ivfuid generatord gladd glfw stbd imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL glm::glm)
    target_include_directories(example_6d PRIVATE ${Stb_INCLUDE_DIR})
else()
endif()


