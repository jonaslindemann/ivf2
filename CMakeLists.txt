project(ivf2)

cmake_minimum_required(VERSION 3.8)

set( LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib )
set( EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin )

set( GLFW_INCLUDE_DIR "E:/Users/Jonas/Development/Libraries/include" )
set( GLM_INCLUDE_DIR "E:/Users/Jonas/Development/Libraries/include" )
set( GML_INCLUDE_DIR "E:/Users/Jonas/Development/Libraries/include" )
set( KHR_INCLUDE_DIR "E:/Users/Jonas/Development/Libraries/include" )
set( STB_INCLUDE_DIR "E:/Users/Jonas/Development/Libraries/include" )
set( GENERATOR_INCLUDE_DIR "E:/Users/Jonas/Development/Libraries/include")

set( GLFW_LIB_DIR "E:/Users/Jonas/Development/Libraries/lib" )
set( GENERATOR_LIB_DIR "E:/Users/Jonas/Development/Libraries/lib" )

set(CMAKE_DEBUG_POSTFIX d)

include_directories( ${PROJECT_SOURCE_DIR}/include ${GLFW_INCLUDE_DIR} ${GLM_INCLUDE_DIR} ${GML_INCLUDE_DIR} ${KHR_INCLUDE_DIR} ${STB_INCLUDE_DIR} ${GENERATOR_INCLUDE_DIR} )
link_directories( ${PROJECT_SOURCE_DIR}/lib ${GLFW_LIB_DIR} ${GENERATOR_LIB_DIR} )

subdirs(src examples)
