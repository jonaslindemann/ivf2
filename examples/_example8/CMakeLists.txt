file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

if (WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_executable(example8d example8.cpp)
        target_link_libraries(example8d ivf2d generatord gladd glfw3d stbd )
    else()
        add_executable(example8 example8.cpp)
        target_link_libraries(example8 ivf2 generator glad glfw3 stb)
    endif()
else()
    add_executable(example8 example8.cpp)
    target_link_libraries(example8 ivf2 generator glad glfw3 stb dl pthread )
endif()


