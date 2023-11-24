INCLUDE_DIRECTORIES(${ivf2_SOURCE_DIR}/include)
FILE(GLOB SOURCE_FILES *.cpp)
FILE(GLOB INCLUDE_FILES ${ivf2_SOURCE_DIR}/include/ivfui/*.h)

ADD_LIBRARY(ivfui ${INCLUDE_FILES} ${SOURCE_FILES})
target_include_directories(ivfui PRIVATE ${GLAD_INCLUDE_DIR} glm::glm)
INSTALL_TARGETS(/lib ivfui)

 