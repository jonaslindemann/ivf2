file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_12d example_12.cpp)
    target_link_libraries(example_12d PRIVATE gladd ivfd ivfuid generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(example_12d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_12d gladd generatord ivfd ivfuid)
else()
    add_executable(example_12 example_12.cpp)
    target_link_libraries(example_12 PRIVATE glad ivf ivfui generator glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(example_12 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_12 glad generator ivf ivfui)
endif() 


