INCLUDE_DIRECTORIES(${ivf2_SOURCE_DIR}/include)
FILE(GLOB SOURCE_FILES *.cpp)
FILE(GLOB INCLUDE_FILES ${ivf2_SOURCE_DIR}/include/ivf/*.h) 

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
ADD_LIBRARY(ivfd ${INCLUDE_FILES} ${SOURCE_FILES})
target_include_directories(ivfd PRIVATE glm::glm)
#target_link_libraries(ivfd generator glad)
add_dependencies(ivfd gladd generatord)
INSTALL_TARGETS(/lib ivfd)
else()
ADD_LIBRARY(ivf ${INCLUDE_FILES} ${SOURCE_FILES})
target_include_directories(ivf PRIVATE glm::glm)
#target_link_libraries(ivf generator glad)
add_dependencies(ivf glad generator)
INSTALL_TARGETS(/lib ivf) 
endif()

 