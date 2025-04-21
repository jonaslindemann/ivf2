file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(extrusions1d extrusions1.cpp)
    target_link_libraries(extrusions1d PRIVATE gladd ivfd ivfuid generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(extrusions1d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(extrusions1d gladd generatord ivfd ivfuid)
    set_target_properties(extrusions1d PROPERTIES FOLDER "examples")
else()
    add_executable(extrusions1 extrusions1.cpp)
    target_link_libraries(extrusions1 PRIVATE glad ivf ivfui generator glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(extrusions1 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(extrusions1 glad generator ivf ivfui)
    set_target_properties(extrusions1 PROPERTIES FOLDER "examples")
endif()


