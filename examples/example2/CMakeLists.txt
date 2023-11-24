file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_2d example_2.cpp example_window_1.h example_window_2.h example_window_1.cpp example_window_2.cpp)
    target_link_libraries(example_2d PRIVATE ivfd ivfuid generatord gladd glfw stbd imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL glm::glm)
    target_include_directories(example_2d PRIVATE ${Stb_INCLUDE_DIR})
else()
endif()


