file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_16d example_16.cpp)
    target_link_libraries(example_16d PRIVATE gladd ivfd ivfuid ivfmathd generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype assimp::assimp) 
    target_include_directories(example_16d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_16d gladd generatord ivfd ivfuid)
else()
    add_executable(example_16 example_16.cpp)
    target_link_libraries(example_16 PRIVATE glad ivf ivfui ivfmath generator glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype assimp::assimp) 
    target_include_directories(example_16 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_16 glad generator ivf ivfui)
endif() 


