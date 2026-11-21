# ivfui - GLFW windowing, input, camera manipulators and ImGui integration

file(GLOB _sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB _headers CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/include/ivfui/*.h)

ivf2_add_library(ivfui PCH pch.h SOURCES ${_sources} ${_headers})

target_link_libraries(ivfui PUBLIC ivf glfw imgui::imgui)
