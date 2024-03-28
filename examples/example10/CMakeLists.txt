file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_10d example_10.cpp)
    target_link_libraries(example_10d PRIVATE gladd ivfd ivfuid generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(example_10d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_10d gladd generatord ivfd ivfuid)
else()
    add_executable(example_10 example_10.cpp)
    target_link_libraries(example_10 PRIVATE glad ivf ivfui generator glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(example_10 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_10 glad generator ivf ivfui)
endif() 


