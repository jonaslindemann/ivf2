file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(deformer1d deformer1.cpp twist_window.h twist_window.cpp bend_window.h bend_window.cpp)
    target_link_libraries(deformer1d PRIVATE ivfuid ivfd ivfmathd generatord glfw gladd imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(deformer1d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(deformer1d gladd generatord ivfd ivfuid)
    set_target_properties(deformer1d PROPERTIES FOLDER "examples")
else()
    add_executable(deformer1 deformer1.cpp twist_window.h twist_window.cpp bend_window.h bend_window.cpp)
    target_link_libraries(deformer1 PRIVATE ivfui ivf ivfmath generator glfw glad imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(deformer1 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(deformer1 glad generator ivf ivfui)
    set_target_properties(deformer1 PROPERTIES FOLDER "examples") 
endif() 


