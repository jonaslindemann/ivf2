file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(selection1d selection1.cpp)
    target_link_libraries(selection1d PRIVATE gladd ivfd ivfuid generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(selection1d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(selection1d gladd generatord ivfd ivfuid)
    set_target_properties(selection1d PROPERTIES FOLDER "examples")
else()
    add_executable(selection1 selection1.cpp)
    target_link_libraries(selection1 PRIVATE glad ivf ivfui generator glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(selection1 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(selection1 glad generator ivf ivfui)
    set_target_properties(selection1 PROPERTIES FOLDER "examples")
endif() 


