file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(text1d text1.cpp text_window.h text_window.cpp)
    target_link_libraries(text1d PRIVATE ivfuid ivfd ivfmathd generatord glfw gladd imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(text1d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(text1d gladd generatord ivfd ivfuid)
    set_target_properties(text1d PROPERTIES FOLDER "examples")
else()
    add_executable(text1 text1.cpp text_window.h text_window.cpp)
    target_link_libraries(text1 PRIVATE ivfui ivf ivfmath generator glfw glad imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(text1 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(text1 glad generator ivf ivfui)
    set_target_properties(text1 PROPERTIES FOLDER "examples")
endif()


