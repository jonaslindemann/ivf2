INCLUDE_DIRECTORIES(${ivf2_SOURCE_DIR}/include)
FILE(GLOB SOURCE_FILES *.cpp)
FILE(GLOB INCLUDE_FILES ${ivf2_SOURCE_DIR}/include/ivfmusic/*.h)

add_library(ivfmusic ${INCLUDE_FILES} ${SOURCE_FILES})
add_library(ivf2::ivfmusic ALIAS ivfmusic)

# ivf2 is the unified interface target defined in the top-level CMakeLists.txt
target_link_libraries(ivfmusic PUBLIC ivf2 nlohmann_json::nlohmann_json)
add_dependencies(ivfmusic ivf ivfui)
target_precompile_headers(ivfmusic PRIVATE pch.h)

if(IVF_WITH_RENOISE)
    find_package(miniz CONFIG REQUIRED)
    find_package(pugixml CONFIG REQUIRED)
    target_compile_definitions(ivfmusic PRIVATE IVF_WITH_RENOISE)
    target_link_libraries(ivfmusic PRIVATE miniz::miniz pugixml::pugixml)
endif()

if(MSVC)
    target_compile_options(ivfmusic PRIVATE /utf-8)
endif()

install_targets(/lib ivfmusic)
set_target_properties(ivfmusic PROPERTIES
    FOLDER "ivf"
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
)
