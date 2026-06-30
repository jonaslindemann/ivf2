cmake_minimum_required(VERSION 3.24)

# Auto-detect vcpkg toolchain file, preferring standalone installations over VS bundled vcpkg
# Check if Visual Studio's bundled vcpkg is being used and override with local installations
set(_VS_VCPKG_DETECTED FALSE)
if(DEFINED CMAKE_TOOLCHAIN_FILE)
    string(FIND "${CMAKE_TOOLCHAIN_FILE}" "Microsoft Visual Studio" _VS_VCPKG_POS)
    if(NOT _VS_VCPKG_POS EQUAL -1)
        set(_VS_VCPKG_DETECTED TRUE)
        message(STATUS "Visual Studio bundled vcpkg detected, checking for local installations...")
    endif()
endif() 

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE OR _VS_VCPKG_DETECTED)
    # Check for VCPKG_ROOT environment variable first
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" 
            CACHE STRING "Vcpkg toolchain file" FORCE)
        message(STATUS "Using vcpkg from VCPKG_ROOT: $ENV{VCPKG_ROOT}")
    # Try common vcpkg installation locations
    elseif(WIN32)
        if(EXISTS "c:/vcpkg/scripts/buildsystems/vcpkg.cmake")
            set(CMAKE_TOOLCHAIN_FILE "c:/vcpkg/scripts/buildsystems/vcpkg.cmake" 
                CACHE STRING "Vcpkg toolchain file" FORCE)
            message(STATUS "Found and using vcpkg at: c:/vcpkg")
        elseif(EXISTS "e:/vcpkg/scripts/buildsystems/vcpkg.cmake")
            set(CMAKE_TOOLCHAIN_FILE "e:/vcpkg/scripts/buildsystems/vcpkg.cmake" 
                CACHE STRING "Vcpkg toolchain file" FORCE)
            message(STATUS "Found and using vcpkg at: e:/vcpkg")
        elseif(_VS_VCPKG_DETECTED)
            message(STATUS "No local vcpkg found, using Visual Studio bundled vcpkg: ${CMAKE_TOOLCHAIN_FILE}")
        endif()
    elseif(UNIX AND NOT APPLE)
        if(EXISTS "$ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake")
            set(CMAKE_TOOLCHAIN_FILE "$ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake" 
                CACHE STRING "Vcpkg toolchain file" FORCE)
            message(STATUS "Found and using vcpkg at: $ENV{HOME}/vcpkg")
        elseif(EXISTS "/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake")
            set(CMAKE_TOOLCHAIN_FILE "/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake" 
                CACHE STRING "Vcpkg toolchain file" FORCE)
            message(STATUS "Found and using vcpkg at: /usr/local/vcpkg")
        endif()
    elseif(APPLE)
        if(EXISTS "$ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake")
            set(CMAKE_TOOLCHAIN_FILE "$ENV{HOME}/vcpkg/scripts/buildsystems/vcpkg.cmake" 
                CACHE STRING "Vcpkg toolchain file" FORCE)
            message(STATUS "Found and using vcpkg at: $ENV{HOME}/vcpkg")
        elseif(EXISTS "/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake")
            set(CMAKE_TOOLCHAIN_FILE "/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake" 
                CACHE STRING "Vcpkg toolchain file" FORCE)
            message(STATUS "Found and using vcpkg at: /usr/local/vcpkg")
        endif()
    endif()
    
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        message(WARNING "vcpkg toolchain file not found. Please set VCPKG_ROOT environment variable or CMAKE_TOOLCHAIN_FILE.")
    endif()
endif()

# Share vcpkg installed packages across all build directories to save disk space
# This puts vcpkg_installed/ in the project root instead of each build directory
set(VCPKG_INSTALLED_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed" CACHE PATH "vcpkg installed directory")

# On macOS, verify the SDK has working std::format support.
# Two known-broken cases:
#   1. Older SDKs set _LIBCPP_HAS_NO_INCOMPLETE_FORMAT to disable <format> entirely.
#   2. Xcode MacOSX14 SDK has a compilation bug in std::format with Apple Clang 15+.
# In either case, fall back to the CommandLineTools SDK which has a correct implementation.
if(APPLE AND NOT CMAKE_OSX_SYSROOT)
    execute_process(
        COMMAND xcrun --sdk macosx --show-sdk-path
        OUTPUT_VARIABLE _IVF_CURRENT_SDK
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    set(_IVF_FORMAT_BROKEN FALSE)
    if(_IVF_CURRENT_SDK AND EXISTS "${_IVF_CURRENT_SDK}/usr/include/c++/v1/__config")
        file(READ "${_IVF_CURRENT_SDK}/usr/include/c++/v1/__config" _IVF_LIBCXX_CONFIG)
        string(FIND "${_IVF_LIBCXX_CONFIG}" "_LIBCPP_HAS_NO_INCOMPLETE_FORMAT" _IVF_FORMAT_DISABLED)
        if(NOT _IVF_FORMAT_DISABLED EQUAL -1)
            set(_IVF_FORMAT_BROKEN TRUE)
        endif()
    endif()
    # Xcode MacOSX14.x SDKs have a broken std::format ABI with Apple Clang 15+
    string(FIND "${_IVF_CURRENT_SDK}" "MacOSX14" _IVF_SDK14_POS)
    if(NOT _IVF_SDK14_POS EQUAL -1)
        set(_IVF_FORMAT_BROKEN TRUE)
    endif()
    if(_IVF_FORMAT_BROKEN)
        message(STATUS "SDK ${_IVF_CURRENT_SDK} has broken std::format — searching for a working CommandLineTools SDK")
        foreach(_IVF_SDK_CANDIDATE
            "/Library/Developer/CommandLineTools/SDKs/MacOSX15.4.sdk"
            "/Library/Developer/CommandLineTools/SDKs/MacOSX15.sdk"
            "/Library/Developer/CommandLineTools/SDKs/MacOSX26.2.sdk"
            "/Library/Developer/CommandLineTools/SDKs/MacOSX26.sdk"
        )
            if(EXISTS "${_IVF_SDK_CANDIDATE}/usr/include/c++/v1/__config")
                file(READ "${_IVF_SDK_CANDIDATE}/usr/include/c++/v1/__config" _IVF_CLT_CONFIG)
                string(FIND "${_IVF_CLT_CONFIG}" "_LIBCPP_HAS_NO_INCOMPLETE_FORMAT" _IVF_CLT_DISABLED)
                if(_IVF_CLT_DISABLED EQUAL -1)
                    set(CMAKE_OSX_SYSROOT "${_IVF_SDK_CANDIDATE}" CACHE PATH "macOS SDK with std::format support" FORCE)
                    message(STATUS "Using SDK: ${_IVF_SDK_CANDIDATE}")
                    break()
                endif()
            endif()
        endforeach()
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
find_package(RtMidi CONFIG REQUIRED)

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
    RtMidi::rtmidi
)

target_include_directories(ivf2 INTERFACE 
    ${PROJECT_SOURCE_DIR}/include
    ${Stb_INCLUDE_DIR}
)

add_subdirectory(examples)  
