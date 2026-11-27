# Web builds (Emscripten): stand-ins for the targets of OpenGL, GLFW and threads.
#
# The browser provides OpenGL ES 3.0 as WebGL 2 and Emscripten its own GLFW for the page's
# canvas, so these targets only add link options. glad loads the GL functions through
# glfwGetProcAddress (RenderContext::create), hence GL_ENABLE_GET_PROC_ADDRESS. No threads:
# pages then need no special HTTP headers (cross-origin isolation) and can be hosted anywhere.
#
# Included by ivf2's CMakeLists.txt and by the installed ivf2Config.cmake (instead of
# find_package(OpenGL/glfw3/Threads)), so applications using find_package(ivf2) get them too.

if(NOT TARGET OpenGL::GL)
    add_library(OpenGL::GL INTERFACE IMPORTED GLOBAL)
    target_link_options(OpenGL::GL INTERFACE
        "SHELL:-s MIN_WEBGL_VERSION=2" "SHELL:-s MAX_WEBGL_VERSION=2" "SHELL:-s GL_ENABLE_GET_PROC_ADDRESS=1")
endif()
if(NOT TARGET glfw)
    add_library(glfw INTERFACE IMPORTED GLOBAL)
    target_link_options(glfw INTERFACE "SHELL:-s USE_GLFW=3")
endif()
if(NOT TARGET Threads::Threads)
    add_library(Threads::Threads INTERFACE IMPORTED GLOBAL)
endif()
