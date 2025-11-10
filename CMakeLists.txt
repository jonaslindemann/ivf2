cmake_minimum_required(VERSION 3.24)

# Auto-detect vcpkg toolchain file if not already set
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    # Check for VCPKG_ROOT environment variable first
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" 
            CACHE STRING "Vcpkg toolchain file")
        message(STATUS "Using vcpkg from VCPKG_ROOT: $ENV{VCPKG_ROOT}")
    # Try common vcpkg installation locations
    elseif(WIN32)
        if(EXISTS "c:/vcpkg/scripts/buildsystems/vcpkg.cmake")
            set(CMAKE_TOOLCHAIN_FILE "c:/vcpkg/scripts/buildsystems/vcpkg.cmake" 
                CACHE STRING "Vcpkg toolchain file")
            message(STATUS "Found vcpkg at: c:/vcpkg")
        elseif(EXISTS "e:/vcpkg/scripts/buildsystems/vcpkg.cmake")
            set(CMAKE_TOOLCHAIN_FILE "e:/vcpkg/scripts/buildsystems/vcpkg.cmake" 
                CACHE STRING "Vcpkg toolchain file")
            message(STATUS "Found vcpkg at: e:/vcpkg")
        endif()
    elseif(UNIX AND NOT APPLE)
        if(EXISTS "$ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake")
            set(CMAKE_TOOLCHAIN_FILE "$ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake" 
                CACHE STRING "Vcpkg toolchain file")
            message(STATUS "Found vcpkg at: $ENV{HOME}/vcpkg")
        elseif(EXISTS "/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake")
            set(CMAKE_TOOLCHAIN_FILE "/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake" 
                CACHE STRING "Vcpkg toolchain file")
            message(STATUS "Found vcpkg at: /usr/local/vcpkg")
        endif()
    elseif(APPLE)
        if(EXISTS "$ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake")
            set(CMAKE_TOOLCHAIN_FILE "$ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake" 
                CACHE STRING "Vcpkg toolchain file")
            message(STATUS "Found vcpkg at: $ENV{HOME}/vcpkg")
        elseif(EXISTS "/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake")
            set(CMAKE_TOOLCHAIN_FILE "/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake" 
                CACHE STRING "Vcpkg toolchain file")
            message(STATUS "Found vcpkg at: /usr/local/vcpkg")
        endif()
    endif()
    
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        message(WARNING "vcpkg toolchain file not found. Please set VCPKG_ROOT environment variable or CMAKE_TOOLCHAIN_FILE.")
    endif()
endif()

project(ivf2) 

enable_language(CXX)
enable_language(C)
set(CMAKE_CXX_STANDARD 20) 

# Enable automatic debug postfix for all targets
set(CMAKE_DEBUG_POSTFIX d)

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
find_package(ReactPhysics3D REQUIRED)
#  target_link_libraries(main PRIVATE ReactPhysics3D::ReactPhysics3D)

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

# Create unified ivf2 interface library target
# This bundles all ivf libraries and dependencies for easier linking
add_library(ivf2 INTERFACE)
add_library(ivf2::ivf2 ALIAS ivf2)

target_link_libraries(ivf2 INTERFACE 
    ivfui 
    ivf 
    ivfmath 
    generator 
    glad 
    imgui::imgui 
    PNG::PNG 
    JPEG::JPEG 
    ZLIB::ZLIB 
    glfw 
    GLEW::GLEW 
    OpenGL::GL 
    Freetype::Freetype
)

target_include_directories(ivf2 INTERFACE 
    ${PROJECT_SOURCE_DIR}/include
    ${Stb_INCLUDE_DIR}
)

add_subdirectory(examples)  
