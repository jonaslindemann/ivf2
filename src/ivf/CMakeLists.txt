INCLUDE_DIRECTORIES(${ivf2_SOURCE_DIR}/include)
FILE(GLOB SOURCE_FILES *.cpp)
FILE(GLOB INCLUDE_FILES ${ivf2_SOURCE_DIR}/include/ivf/*.h) 

add_library(ivf ${INCLUDE_FILES} ${SOURCE_FILES})
target_include_directories(ivf PRIVATE ${GLM_INCLUDE_DIRS})
add_dependencies(ivf glad generator)
install_targets(/lib ivf)
set_target_properties(ivf PROPERTIES 
    FOLDER "ivf" 
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
)   
      