file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(animation2d animation2.cpp lissajou_window.h lissajou_window.cpp)
    target_link_libraries(animation2d PRIVATE gladd ivfd ivfuid generatord glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(animation2d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(animation2d gladd generatord ivfd ivfuid)
    set_target_properties(animation2d PROPERTIES FOLDER "examples")
else()
    add_executable(animation2 animation2.cpp lissajou_window.h lissajou_window.cpp)
    target_link_libraries(animation2 PRIVATE glad ivf ivfui generator glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(animation2 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(animation2 glad generator ivf ivfui)
    set_target_properties(animation2 PROPERTIES FOLDER "examples")
endif()  


