file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(animation3d animation3.cpp)
    target_link_libraries(animation3d PRIVATE gladd ivfd ivfuid ivfmathd generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(animation3d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(animation3d gladd generatord ivfd ivfuid)
    set_target_properties(animation3d PROPERTIES FOLDER "examples")
else()
    add_executable(animation3 animation3.cpp)
    target_link_libraries(animation3 PRIVATE glad ivf ivfui ivfmath generator glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(animation3 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(animation3 glad generator ivf ivfui)
    set_target_properties(animation3 PROPERTIES FOLDER "examples")
endif() 


