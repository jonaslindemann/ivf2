file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_executable(example7d example7.cpp)
        target_link_libraries(example7d ivf2d generatord gladd glfw3d stbd )
    else()
        add_executable(example7 example7.cpp)
        target_link_libraries(example7 ivf2 generator glad glfw3 stb)
    endif()
else()
    add_executable(example7 example7.cpp)
    target_link_libraries(example7 ivf2 generator glad glfw3 stb dl pthread )
endif()


