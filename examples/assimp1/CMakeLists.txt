file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

add_ivf2_example(assimp1 SOURCES ${SOURCE_FILES} ${INCLUDE_FILES} EXTRA_LIBS assimp::assimp)
