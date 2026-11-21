# ivfmath - keyframe/spline interpolation and spherical math

file(GLOB _sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB _headers CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/include/ivfmath/*.h)

ivf2_add_library(ivfmath PCH pch.h SOURCES ${_sources} ${_headers})
