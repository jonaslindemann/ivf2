file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

add_ivf2_example(assimp1 SOURCES ${SOURCE_FILES} ${INCLUDE_FILES})

# Third-party file: not held to ivf2's warning level
set_source_files_properties(ImGuiFileDialog.cpp PROPERTIES
    SKIP_PRECOMPILE_HEADERS ON
    COMPILE_OPTIONS "$<IF:$<CXX_COMPILER_ID:MSVC>,/W0,-w>")
