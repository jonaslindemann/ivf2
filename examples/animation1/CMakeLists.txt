file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(animation1d animation1.cpp)
    target_link_libraries(animation1d PRIVATE gladd ivfd ivfuid generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(animation1d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(animation1d gladd generatord ivfd ivfuid)
    set_target_properties(animation1d PROPERTIES FOLDER "examples")
else()
    add_executable(animation1 animation1.cpp)
    target_link_libraries(animation1 PRIVATE glad ivf ivfui generator glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(animation1 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(animation1 glad generator ivf ivfui)
    set_target_properties(animation1 PROPERTIES FOLDER "examples")
endif() 


