file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(solar_systemd base.h base.cpp planet.h planet.cpp solar_panel.h solar_panel.cpp solar_system.h solar_system.cpp solar_window.h solar_window.cpp sun.h sun.cpp utils.h utils.cpp vec3d.h vec3d.cpp main.cpp)
    target_link_libraries(solar_systemd PRIVATE ivfuid ivfd ivfmathd generatord glfw gladd imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(solar_systemd PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(solar_systemd gladd generatord ivfd ivfuid)
    set_target_properties(solar_systemd PROPERTIES FOLDER "examples")
else()
    add_executable(solar_system base.h base.cpp planet.h planet.cpp solar_panel.h solar_panel.cpp solar_system.h solar_system.cpp solar_window.h solar_window.cpp sun.h sun.cpp utils.h utils.cpp vec3d.h vec3d.cpp main.cpp)
    target_link_libraries(solar_system PRIVATE ivfui ivf ivfmath generator glfw glad imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL Freetype::Freetype) 
    target_include_directories(solar_system PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(solar_system glad generator ivf ivfui)
    set_target_properties(solar_system PROPERTIES FOLDER "examples")
endif() 


