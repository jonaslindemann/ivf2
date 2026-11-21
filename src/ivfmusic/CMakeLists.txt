# ivfmusic - music-driven scenes (song transport, automation, Renoise scores)

file(GLOB _sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB _headers CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/include/ivfmusic/*.h)

ivf2_add_library(ivfmusic PCH pch.h SOURCES ${_sources} ${_headers})

target_link_libraries(ivfmusic PUBLIC ivfui PRIVATE nlohmann_json::nlohmann_json)

if(IVF_WITH_RENOISE)
    target_compile_definitions(ivfmusic PRIVATE IVF_WITH_RENOISE)
    target_link_libraries(ivfmusic PRIVATE miniz::miniz pugixml::pugixml)
endif()
