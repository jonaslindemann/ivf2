file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(mesh2d mesh2.cpp )
    target_link_libraries(mesh2d PRIVATE ivfuid ivfd ivfmathd generatord glfw gladd imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(mesh2d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(mesh2d gladd generatord ivfd ivfuid)
    set_target_properties(mesh2d PROPERTIES FOLDER "examples")
else()
    add_executable(mesh2 mesh2.cpp)
    target_link_libraries(mesh2 PRIVATE ivfui ivf ivfmath generator glfw glad imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(mesh2 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(mesh2 glad generator ivf ivfui)
    set_target_properties(mesh2 PROPERTIES FOLDER "examples") 
endif() 


