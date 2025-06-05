cmake_minimum_required(VERSION 3.24)

project(ivf2) 

enable_language(CXX)
enable_language(C)
set(CMAKE_CXX_STANDARD 20)

set( LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib )
set( EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin )

IF(MSVC)
  SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
ENDIF()

find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED) 
find_package(glew REQUIRED)
find_package(OpenGL REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
#find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED) 
find_package(Freetype REQUIRED) 
find_package(assimp CONFIG REQUIRED) 
find_package(nlohmann_json CONFIG REQUIRED)

#if(WIN32)
#    set(CMAKE_DEBUG_POSTFIX d)
#endif()

if (APPLE OR UNIX)
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_compile_definitions(_DEBUG)
	endif()
endif()

include_directories( ${PROJECT_SOURCE_DIR}/include ${GLM_DIR} ${GENERATOR_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS} ${JPEG_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${STB_DIR} ${Stb_INCLUDE_DIR} ${IMGUI_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})

add_subdirectory(src)
add_subdirectory(examples)  
