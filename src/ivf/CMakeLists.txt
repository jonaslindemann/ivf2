# ivf - core scene graph, rendering, primitives, materials, effects, animation

file(GLOB _sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB _headers CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/include/ivf/*.h)

ivf2_add_library(ivf PCH pch.h SOURCES ${_sources} ${_headers})

target_link_libraries(ivf
    PUBLIC
        glad
        generator
        ivfmath
        OpenGL::GL
        imgui::imgui
        Freetype::Freetype
        assimp::assimp
        nlohmann_json::nlohmann_json
        RtMidi::rtmidi
    PRIVATE
        Threads::Threads
        ${CMAKE_DL_LIBS})

# stb is a private implementation detail of image_io.cpp (see ivf/image_io.h).
target_include_directories(ivf PRIVATE ${Stb_INCLUDE_DIR})
if(IVF2_MINIAUDIO_INCLUDE_DIR)
    target_include_directories(ivf PRIVATE ${IVF2_MINIAUDIO_INCLUDE_DIR})
endif()

# image_io.cpp must not see the PCH: an earlier stb include would bypass its
# STB_*_STATIC / *_IMPLEMENTATION defines.
set_source_files_properties(image_io.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
