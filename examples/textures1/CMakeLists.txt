file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(textures1d textures1.cpp)
    target_link_libraries(textures1d PRIVATE ivfuid ivfd ivfmathd generatord glfw gladd imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL)
    target_include_directories(textures1d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(textures1d gladd generatord ivfd ivfuid)
    set_target_properties(textures1d PROPERTIES FOLDER "examples")
else()
    add_executable(textures1 textures1.cpp)
    target_link_libraries(textures1 PRIVATE ivfui ivf ivfmath generator glfw glad imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL)
    target_include_directories(textures1 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(textures1 glad generator ivf ivfui)
    set_target_properties(textures1 PROPERTIES FOLDER "examples")
endif()


