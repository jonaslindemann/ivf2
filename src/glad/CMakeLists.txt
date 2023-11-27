include_directories(${ivf2_SOURCE_DIR}/include)
file(GLOB SOURCE_FILES *.c)
file(GLOB INCLUDE_FILES ${ivf2_SOURCE_DIR}/include/glad/*.h)

add_library(glad ${INCLUDE_FILES} ${SOURCE_FILES})
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)
install_targets(/lib glad)

