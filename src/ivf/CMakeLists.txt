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
        nlohmann_json::nlohmann_json
    PRIVATE
        Threads::Threads
        ${CMAKE_DL_LIBS})

# Optional dependencies are private: no public ivf header includes them (see
# ivf/config.h for the IVF2_HAS_* switches the sources check).
if(IVF2_WITH_ASSIMP)
    target_link_libraries(ivf PRIVATE assimp::assimp)
endif()
if(IVF2_WITH_FREETYPE)
    target_link_libraries(ivf PRIVATE Freetype::Freetype)
endif()
if(IVF2_WITH_AUDIO)
    target_include_directories(ivf PRIVATE ${IVF2_MINIAUDIO_INCLUDE_DIR})
endif()
if(IVF2_WITH_MIDI)
    target_link_libraries(ivf PRIVATE RtMidi::rtmidi)
endif()

# stb is a private implementation detail of image_io.cpp (see ivf/image_io.h).
target_include_directories(ivf PRIVATE ${Stb_INCLUDE_DIR})

# image_io.cpp must not see the PCH: an earlier stb include would bypass its
# STB_*_STATIC / *_IMPLEMENTATION defines.
set_source_files_properties(image_io.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
