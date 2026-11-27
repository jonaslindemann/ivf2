# Checks for web builds (preset wasm). Open the page in a browser (or headless Chrome in CI):
# shader_check compiles every built-in shader with WebGL 2 and reports failures in the log
# and the page title.

add_executable(shader_check shader_check.cpp)
target_link_libraries(shader_check PRIVATE ivf2::ivf2)
ivf2_apply_compiler_settings(shader_check)
set_target_properties(shader_check PROPERTIES FOLDER "tests")
ivf2_web_executable(shader_check)
