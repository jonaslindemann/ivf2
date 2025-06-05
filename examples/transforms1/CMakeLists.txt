file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(transforms1d transforms1.cpp)
    target_link_libraries(transforms1d PRIVATE ivfuid ivfd ivfmathd generatord glfw gladd imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(transforms1d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(transforms1d gladd generatord ivfd ivfuid)
    set_target_properties(transforms1d PROPERTIES FOLDER "examples")
else()
    add_executable(transforms1 transforms1.cpp)
    target_link_libraries(transforms1 PRIVATE ivfui ivf ivfmath generator glfw glad imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(transforms1 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(transforms1 glad generator ivf ivfui)
    set_target_properties(transforms1 PROPERTIES FOLDER "examples")
endif()


