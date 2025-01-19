file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_14d example_14.cpp)
    target_link_libraries(example_14d PRIVATE gladd ivfd ivfuid ivfmathd generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype assimp::assimp) 
    target_include_directories(example_14d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_14d gladd generatord ivfd ivfuid)
else()
    add_executable(example_14 example_14.cpp)
    target_link_libraries(example_14 PRIVATE glad ivf ivfui ivfmath generator glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype assimp::assimp) 
    target_include_directories(example_14 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_14 glad generator ivf ivfui)
endif() 


