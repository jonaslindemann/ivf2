file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_executable(example1d example1.cpp)
        target_link_libraries(example1d ivf2d generatord gladd glfw3d stbd )
    else()
        add_executable(example1 example1.cpp)
        target_link_libraries(example1 ivf2 generator glad glfw3 stb)
    endif()
else()
    add_executable(example1 example1.cpp)
    target_link_libraries(example1 ivf2 generator glad glfw3 stb dl pthread )
endif()


