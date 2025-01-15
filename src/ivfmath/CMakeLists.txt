INCLUDE_DIRECTORIES(${ivf2_SOURCE_DIR}/include)
FILE(GLOB SOURCE_FILES *.cpp)
FILE(GLOB INCLUDE_FILES ${ivf2_SOURCE_DIR}/include/ivfmath/*.h) 

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_library(ivfmathd ${INCLUDE_FILES} ${SOURCE_FILES})
    target_include_directories(ivfmathd PRIVATE ${GLM_INCLUDE_DIRS})
    install_targets(/lib ivfmathd)
else()
    add_library(ivfmath ${INCLUDE_FILES} ${SOURCE_FILES})
    target_include_directories(ivfmath PRIVATE glm::glm)
    install_targets(/lib ivfmath)  
endif()   

 