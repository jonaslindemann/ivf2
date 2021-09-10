file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_executable(example6d example6.cpp textwindow.h textwindow.cpp)
        target_link_libraries(example6d ivf2d generatord gladd glfw3d imguid stbd freetype)
    else()
        add_executable(example6 example6.cpp textwindow.h textwindow.cpp)
        target_link_libraries(example6 ivf2 generator glad glfw3 imgui stb freetype )
    endif()
else()
    add_executable(example6 example6.cpp textwindow.h textwindow.cpp)
    target_link_libraries(example6 ivf2 generator glad glfw3 imgui stb freetype dl pthread )
endif()


