INCLUDE_DIRECTORIES(${ivf2_SOURCE_DIR}/include) 
FILE(GLOB SOURCE_FILES *.cpp) 
FILE(GLOB INCLUDE_FILES ${ivf2_SOURCE_DIR}/include/ivfui/*.h)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_library(ivfuid ${INCLUDE_FILES} ${SOURCE_FILES})
	target_include_directories(ivfuid PRIVATE glm::glm)
	#target_link_libraries(ivfuid generatord gladd ivfd)
	add_dependencies(ivfuid ivfd)
	install_targets(/lib ivfuid)
	set_target_properties(ivfuid PROPERTIES FOLDER "ivf")
else()
	ADD_LIBRARY(ivfui ${INCLUDE_FILES} ${SOURCE_FILES})
	target_include_directories(ivfui PRIVATE glm::glm)
	#target_link_libraries(ivfui generator glad ivf)
	add_dependencies(ivfui ivf)
	INSTALL_TARGETS(/lib ivfui)
	set_target_properties(ivfui PROPERTIES FOLDER "ivf")
endif()    
