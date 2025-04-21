file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(lighting1d lighting1.cpp)
    target_link_libraries(lighting1d PRIVATE gladd ivfd ivfuid generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype)
    target_include_directories(lighting1d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(lighting1d gladd generatord ivfd ivfuid)
    set_target_properties(lighting1d PROPERTIES FOLDER "examples")
else()
    add_executable(lighting1 lighting1.cpp)
    target_link_libraries(lighting1 PRIVATE ivf ivfui generator glad glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype)
    target_include_directories(lighting1 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(lighting1 glad generator ivf ivfui)
    set_target_properties(lighting1 PROPERTIES FOLDER "examples")
endif()


