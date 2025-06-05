file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(mesh1d mesh1.cpp)
    target_link_libraries(mesh1d PRIVATE ivfuid ivfd ivfmathd generatord glfw gladd imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(mesh1d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(mesh1d gladd generatord ivfd ivfuid)
    set_target_properties(mesh1d PROPERTIES FOLDER "examples")
else()
    add_executable(mesh1 mesh1.cpp)
    target_link_libraries(mesh1 PRIVATE ivfui ivf ivfmath generator glfw glad imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(mesh1 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(mesh1 glad generator ivf ivfui)
    set_target_properties(mesh1 PROPERTIES FOLDER "examples")
endif() 


