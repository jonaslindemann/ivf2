INCLUDE_DIRECTORIES(${ivf2_SOURCE_DIR}/include)
FILE(GLOB SOURCE_FILES *.cpp)
FILE(GLOB INCLUDE_FILES ${ivf2_SOURCE_DIR}/include/ivfmath/*.h) 

add_library(ivfmath ${INCLUDE_FILES} ${SOURCE_FILES})
target_include_directories(ivfmath PRIVATE ${GLM_INCLUDE_DIRS})
install_targets(/lib ivfmath)
set_target_properties(ivfmath PROPERTIES FOLDER "ivf")
    

 