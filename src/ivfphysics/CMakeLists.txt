# ivfphysics - rigid body physics for scene nodes (ReactPhysics3D, private)

file(GLOB _sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB _headers CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/include/ivfphysics/*.h)

ivf2_add_library(ivfphysics FOLDER ivf SOURCES ${_sources} ${_headers} ${CMAKE_CURRENT_SOURCE_DIR}/engine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/convex_hull.h)

target_link_libraries(ivfphysics PUBLIC ivf PRIVATE ReactPhysics3D::ReactPhysics3D)
