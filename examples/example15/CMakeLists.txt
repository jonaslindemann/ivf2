file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_15d grid_layout.h grid_layout.cpp example_15.cpp)
    target_link_libraries(example_15d PRIVATE gladd ivfd ivfuid ivfmathd generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype assimp::assimp) 
    target_include_directories(example_15d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_15d gladd generatord ivfd ivfuid)
else()
    add_executable(example_15 grid_layout.h grid_layout.cpp example_15.cpp)
    target_link_libraries(example_15 PRIVATE glad ivf ivfui ivfmath generator glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype assimp::assimp) 
    target_include_directories(example_15 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_15 glad generator ivf ivfui)
endif() 


