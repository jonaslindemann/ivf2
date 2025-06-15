file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(deformer2d deformer2.cpp )
    target_link_libraries(deformer2d PRIVATE ivfuid ivfd ivfmathd generatord glfw gladd imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(deformer2d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(deformer2d gladd generatord ivfd ivfuid)
    set_target_properties(deformer2d PROPERTIES FOLDER "examples")
else()
    add_executable(deformer2 deformer2.cpp)
    target_link_libraries(deformer2 PRIVATE ivfui ivf ivfmath generator glfw glad imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(deformer2 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(deformer2 glad generator ivf ivfui)
    set_target_properties(deformer2 PROPERTIES FOLDER "examples") 
endif() 


