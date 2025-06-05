file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(effects1d effects1.cpp)
    target_link_libraries(effects1d PRIVATE ivfuid ivfd ivfmathd generatord glfw gladd imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype assimp::assimp) 
    target_include_directories(effects1d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(effects1d gladd generatord ivfd ivfuid)
    set_target_properties(effects1d PROPERTIES FOLDER "examples")
else()
    add_executable(effects1 effects1.cpp)
    target_link_libraries(effects1 PRIVATE ivfui ivf ivfmath generator glfw glad imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype assimp::assimp) 
    target_include_directories(effects1 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(effects1 glad generator ivf ivfui)
    set_target_properties(effects1 PROPERTIES FOLDER "examples")
endif() 


