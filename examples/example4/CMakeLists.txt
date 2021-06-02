file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_executable(example4d example4.cpp)
        target_link_libraries(example4d ivf2d generatord gladd glfw3d imguid stbd)
    else()
        add_executable(example4 example4.cpp )
        target_link_libraries(example4 ivf2 generator glad glfw3 imgui stb)
    endif()
else()
    add_executable(example4 example4.cpp)
    target_link_libraries(example4 ivf2 generator glad glfw3 imgui stb dl pthread )
endif()


