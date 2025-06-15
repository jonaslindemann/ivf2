file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(deformer3d deformer3.cpp )
    target_link_libraries(deformer3d PRIVATE ivfuid ivfd ivfmathd generatord glfw gladd imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(deformer3d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(deformer3d gladd generatord ivfd ivfuid)
    set_target_properties(deformer3d PROPERTIES FOLDER "examples")
else()
    add_executable(deformer3 deformer3.cpp)
    target_link_libraries(deformer3 PRIVATE ivfui ivf ivfmath generator glfw glad imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(deformer3 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(deformer3 glad generator ivf ivfui)
    set_target_properties(deformer3 PROPERTIES FOLDER "examples") 
endif() 


