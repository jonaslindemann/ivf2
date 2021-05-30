file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_executable(example2d example2.cpp examplewindow1.cpp examplewindow2.cpp)
        target_link_libraries(example2d ivf2d generatord gladd glfw3d imguid stbd )
    else()
        add_executable(example2 example2.cpp examplewindow1.cpp examplewindow2.cpp)
        target_link_libraries(example2 ivf2 generator glad glfw3 imgui stb )
    endif()
else()
    add_executable(example2 example2.cpp)
    target_link_libraries(example2 ivf2 generator glad glfw3 imgui stb dl pthread )
endif()


