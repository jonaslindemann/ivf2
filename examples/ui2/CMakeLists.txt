file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(ui2d ui2.cpp)
    target_link_libraries(ui2d PRIVATE ivfuid ivfd ivfmathd generatord glfw gladd imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL)
    target_include_directories(ui2d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(ui2d gladd generatord ivfd ivfuid)
    set_target_properties(ui2d PROPERTIES FOLDER "examples")
else()
    add_executable(ui2 ui2.cpp)
    target_link_libraries(ui2 PRIVATE ivfui ivf ivfmath generator glfw glad imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL)
    target_include_directories(ui2 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(ui2 glad generator ivf ivfui)
    set_target_properties(ui2 PROPERTIES FOLDER "examples") 
endif()


