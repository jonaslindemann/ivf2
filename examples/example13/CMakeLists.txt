file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_13d example_13.cpp)
    target_link_libraries(example_13d PRIVATE gladd ivfd ivfuid ivfmathd generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(example_13d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_13d gladd generatord ivfd ivfuid)
else()
    add_executable(example_13 example_13.cpp)
    target_link_libraries(example_13 PRIVATE glad ivf ivfui ivfmath generator glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(example_13 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_13 glad generator ivf ivfui)
endif() 


