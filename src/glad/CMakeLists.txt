# glad - OpenGL function loader (generated, bundled)

ivf2_add_library(glad FOLDER "depends" SOURCES
    glad.c
    ${PROJECT_SOURCE_DIR}/include/glad/glad.h
    ${PROJECT_SOURCE_DIR}/include/KHR/khrplatform.h)

set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(glad PUBLIC OpenGL::GL PRIVATE ${CMAKE_DL_LIBS})
