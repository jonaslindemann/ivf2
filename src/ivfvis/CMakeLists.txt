# ivfvis - scientific visualisation (data sets, colormaps, color bars, ...)

file(GLOB _sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB _headers CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/include/ivfvis/*.h)

ivf2_add_library(ivfvis FOLDER ivf SOURCES ${_sources} ${_headers} ${CMAKE_CURRENT_SOURCE_DIR}/colormap_data.h)

target_link_libraries(ivfvis PUBLIC ivf)

# VTK XML files (.vtu, .vti): XML parsing and zlib decompression, private to ivfvis
if(IVF2_WITH_VTK_XML)
    target_compile_definitions(ivfvis PRIVATE IVFVIS_WITH_VTK_XML)
    target_link_libraries(ivfvis PRIVATE miniz::miniz pugixml::pugixml)
endif()
