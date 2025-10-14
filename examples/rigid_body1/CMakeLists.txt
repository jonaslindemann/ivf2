file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(rigid_body1d rigid_body1.cpp physics_object.cpp physics_box.cpp)
    target_link_libraries(rigid_body1d PRIVATE ivfuid ivfd generatord glfw gladd imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype ReactPhysics3D::ReactPhysics3D)
    target_include_directories(rigid_body1d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(rigid_body1d gladd generatord ivfd ivfuid)
    set_target_properties(rigid_body1d PROPERTIES FOLDER "examples")
else()
    add_executable(rigid_body1 rigid_body1.cpp physics_object.cpp physics_box.cpp)
    target_link_libraries(rigid_body1 PRIVATE ivfui ivf generator glfw glad imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype ReactPhysics3D::ReactPhysics3D)
    target_include_directories(rigid_body1 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(rigid_body1 glad generator ivf ivfui)
    set_target_properties(rigid_body1 PROPERTIES FOLDER "examples")
endif()


