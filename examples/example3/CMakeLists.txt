file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_3d example_3.cpp)
    target_link_libraries(example_3d PRIVATE ivfd ivfuid generatord gladd glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL)
    target_include_directories(example_3d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_3d gladd generatord ivfd ivfuid)
else()
    add_executable(example_3 example_3.cpp)
    target_link_libraries(example_3 PRIVATE ivf ivfui generator glad glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL)
    target_include_directories(example_3 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_3 glad generator ivf ivfui)
endif()


