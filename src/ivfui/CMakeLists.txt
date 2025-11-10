INCLUDE_DIRECTORIES(${ivf2_SOURCE_DIR}/include) 
FILE(GLOB SOURCE_FILES *.cpp) 
FILE(GLOB INCLUDE_FILES ${ivf2_SOURCE_DIR}/include/ivfui/*.h)

add_library(ivfui ${INCLUDE_FILES} ${SOURCE_FILES})
target_include_directories(ivfui PRIVATE glm::glm)
add_dependencies(ivfui ivf)
install_targets(/lib ivfui)
set_target_properties(ivfui PROPERTIES 
    FOLDER "ivf"
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
)
     
