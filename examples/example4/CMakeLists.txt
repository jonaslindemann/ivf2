file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_4d example_4.cpp)
    target_link_libraries(example_4d PRIVATE ivfd ivfuid generatord gladd glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL glm::glm)
    target_include_directories(example_4d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_4d gladd generatord ivfd ivfuid)
else()
    add_executable(example_4 example_4.cpp)
    target_link_libraries(example_4 PRIVATE ivf ivfui generator glad glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL glm::glm)
    target_include_directories(example_4 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_4 glad generator ivf ivfui)
endif()


