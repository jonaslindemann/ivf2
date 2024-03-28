file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_9d example_9.cpp)
    target_link_libraries(example_9d PRIVATE gladd ivfd ivfuid generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(example_9d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_9d gladd generatord ivfd ivfuid)
else()
    add_executable(example_9 example_9.cpp)
    target_link_libraries(example_9 PRIVATE glad ivf ivfui generator glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(example_9 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_9 glad generator ivf ivfui)
endif() 


