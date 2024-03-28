file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_1d example_1.cpp)
    target_link_libraries(example_1d PRIVATE ivfd ivfuid generatord gladd glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL)
    target_include_directories(example_1d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_1d gladd generatord ivfd ivfuid)
else()
    add_executable(example_1 example_1.cpp)
    target_link_libraries(example_1 PRIVATE ivf ivfui generator glad glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL)
    target_include_directories(example_1 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_1 glad generator ivf ivfui  )
endif()


