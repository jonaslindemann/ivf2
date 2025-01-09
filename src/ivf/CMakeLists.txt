INCLUDE_DIRECTORIES(${ivf2_SOURCE_DIR}/include)
FILE(GLOB SOURCE_FILES *.cpp)
FILE(GLOB INCLUDE_FILES ${ivf2_SOURCE_DIR}/include/ivf/*.h) 

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_library(ivfd ${INCLUDE_FILES} ${SOURCE_FILES})
    #target_include_directories(ivfd PRIVATE ${GLM_INCLUDE_DIRS})
    target_include_directories(ivfd PRIVATE ${GLM_INCLUDE_DIRS})
    add_dependencies(ivfd gladd generatord)
    install_targets(/lib ivfd)
else()
    add_library(ivf ${INCLUDE_FILES} ${SOURCE_FILES})
    target_include_directories(ivf PRIVATE glm::glm)
    add_dependencies(ivf glad generator)
    install_targets(/lib ivf)  
endif()   

 