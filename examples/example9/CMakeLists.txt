file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(example_9d example_9.cpp)
    target_link_libraries(example_9d PRIVATE generatord gladd ivfd ivfuid glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL glm::glm Freetype::Freetype) 
    target_include_directories(example_9d PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_9d gladd generatord ivfd ivfuid)
else()
    add_executable(example_9 example_9.cpp)
    target_link_libraries(example_9 PRIVATE generator glad ivf ivfui glfw imgui::imgui PNG::PNG JPEG::JPEG ZLIB::ZLIB glfw GLEW::GLEW OpenGL::GL glm::glm Freetype::Freetype) 
    target_include_directories(example_9 PRIVATE ${Stb_INCLUDE_DIR})
    add_dependencies(example_9 glad generator ivf ivfui)
endif() 


