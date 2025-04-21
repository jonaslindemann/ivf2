file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(assimp1d assimp1.cpp)
    target_link_libraries(assimp1d PRIVATE gladd ivfd ivfuid ivfmathd generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype assimp::assimp) 
    target_include_directories(assimp1d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(assimp1d gladd generatord ivfd ivfuid)
    set_target_properties(assimp1d PROPERTIES FOLDER "examples")
else()
    add_executable(assimp1 assimp1.cpp)
    target_link_libraries(assimp1 PRIVATE glad ivf ivfui ivfmath generator glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype assimp::assimp) 
    target_include_directories(assimp1 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(assimp1 glad generator ivf ivfui)
    set_target_properties(assimp1 PROPERTIES FOLDER "examples")
endif() 


