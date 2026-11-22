file(GLOB SOURCE_FILES *.cpp)
file(GLOB INCLUDE_FILES *.h)

add_ivf2_example(assimp1 SOURCES ${SOURCE_FILES} ${INCLUDE_FILES})

# Third-party file: not held to ivf2's warning level on GCC/Clang. (On MSVC per-file
# warning options do not apply through the shared precompiled header, so it is kept
# warning-free at /W4 instead.)
set_source_files_properties(ImGuiFileDialog.cpp PROPERTIES
    COMPILE_OPTIONS "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>")
