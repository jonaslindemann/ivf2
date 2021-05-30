project(ivf2)

cmake_minimum_required(VERSION 3.8)

set( LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib )
set( EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin )

if (WIN32)
    message("-- Win32 build")
    set( GLFW_INCLUDE_DIR "../ivf2-deps/include" )
    set( GLM_INCLUDE_DIR "../ivf2-deps/include" )
    set( GML_INCLUDE_DIR "../ivf2-deps/include" )
    set( KHR_INCLUDE_DIR "../ivf2-deps/include" )
    set( STB_INCLUDE_DIR "../ivf2-deps/include" )
    set( GENERATOR_INCLUDE_DIR "../ivf2-deps/include")

    set( GLFW_LIB_DIR "../ivf2-deps/lib" )
    set( GENERATOR_LIB_DIR "../ivf2-deps/lib" )
else()
    message("-- Posix build")
    set( GLFW_INCLUDE_DIR "../ivf2-deps/include" )
    set( GLM_INCLUDE_DIR "../ivf2-deps/include" )
    set( GML_INCLUDE_DIR "../ivf2-deps/include" )
    set( KHR_INCLUDE_DIR "../ivf2-deps/include" )
    set( STB_INCLUDE_DIR "../ivf2-deps/include" )
    set( GENERATOR_INCLUDE_DIR "../ivf2-deps/include")

    set( GLFW_LIB_DIR "../ivf2-deps/lib64" )
    set( GENERATOR_LIB_DIR "../ivf2-deps/lib64" )
endif()

if(WIN32)
    set(CMAKE_DEBUG_POSTFIX d)
endif()

include_directories( ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/include/imgui ${GLFW_INCLUDE_DIR} ${GLM_INCLUDE_DIR} ${GML_INCLUDE_DIR} ${KHR_INCLUDE_DIR} ${STB_INCLUDE_DIR} ${GENERATOR_INCLUDE_DIR} )
link_directories( ${PROJECT_SOURCE_DIR}/lib ${GLFW_LIB_DIR} ${GENERATOR_LIB_DIR} )

subdirs(src examples)
