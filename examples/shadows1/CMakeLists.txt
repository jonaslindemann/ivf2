file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(shadows1d shadows1.cpp)
    target_link_libraries(shadows1d PRIVATE gladd ivfd ivfuid ivfmathd generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype assimp::assimp) 
    target_include_directories(shadows1d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(shadows1d gladd generatord ivfd ivfuid)
    set_target_properties(shadows1d PROPERTIES FOLDER "examples")
else()
    add_executable(shadows1 shadows1.cpp)
    target_link_libraries(shadows1 PRIVATE glad ivf ivfui ivfmath generator glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype assimp::assimp) 
    target_include_directories(shadows1 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(shadows1 glad generator ivf ivfui)
    set_target_properties(shadows1 PROPERTIES FOLDER "examples")
endif() 


