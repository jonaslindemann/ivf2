# IvfVcpkgBootstrap.cmake
#
# Included from the top-level CMakeLists.txt *before* project(). Locates a vcpkg
# toolchain when the user did not pass one (via -DCMAKE_TOOLCHAIN_FILE or a
# preset). Does nothing when ivf2 is consumed as a subproject.
#
# Resolution order:
#   1. An explicit CMAKE_TOOLCHAIN_FILE (unless it is the Visual Studio bundled
#      vcpkg and a standalone installation exists).
#   2. $VCPKG_ROOT (unless it points at the Visual Studio bundled vcpkg).
#   3. Common standalone locations (c:/vcpkg, e:/vcpkg, ~/vcpkg, /usr/local/vcpkg, /opt/vcpkg).
#   4. The Visual Studio bundled vcpkg as a last resort (works because vcpkg.json
#      declares a builtin-baseline).

if(DEFINED IVF2_VCPKG_BOOTSTRAP_DONE)
    return()
endif()
set(IVF2_VCPKG_BOOTSTRAP_DONE TRUE)

function(_ivf2_is_vs_vcpkg path out)
    string(FIND "${path}" "Microsoft Visual Studio" _pos)
    if(_pos EQUAL -1)
        set(${out} FALSE PARENT_SCOPE)
    else()
        set(${out} TRUE PARENT_SCOPE)
    endif()
endfunction()

set(_ivf2_vs_fallback "")

if(DEFINED CMAKE_TOOLCHAIN_FILE)
    _ivf2_is_vs_vcpkg("${CMAKE_TOOLCHAIN_FILE}" _tc_is_vs)
    if(NOT _tc_is_vs)
        return() # user knows what they want
    endif()
    set(_ivf2_vs_fallback "${CMAKE_TOOLCHAIN_FILE}")
endif()

set(_ivf2_candidates "")
if(DEFINED ENV{VCPKG_ROOT})
    file(TO_CMAKE_PATH "$ENV{VCPKG_ROOT}" _env_root)
    _ivf2_is_vs_vcpkg("${_env_root}" _env_is_vs)
    if(_env_is_vs)
        if(NOT _ivf2_vs_fallback)
            set(_ivf2_vs_fallback "${_env_root}/scripts/buildsystems/vcpkg.cmake")
        endif()
    else()
        list(APPEND _ivf2_candidates "${_env_root}")
    endif()
endif()

if(WIN32 OR CMAKE_HOST_WIN32)
    list(APPEND _ivf2_candidates "c:/vcpkg" "e:/vcpkg" "c:/src/vcpkg")
else()
    list(APPEND _ivf2_candidates "$ENV{HOME}/vcpkg" "/usr/local/vcpkg" "/opt/vcpkg")
endif()

foreach(_root IN LISTS _ivf2_candidates)
    if(EXISTS "${_root}/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "${_root}/scripts/buildsystems/vcpkg.cmake"
            CACHE FILEPATH "vcpkg toolchain file" FORCE)
        message(STATUS "ivf2: using vcpkg at ${_root}")
        return()
    endif()
endforeach()

if(_ivf2_vs_fallback)
    set(CMAKE_TOOLCHAIN_FILE "${_ivf2_vs_fallback}" CACHE FILEPATH "vcpkg toolchain file" FORCE)
    message(STATUS "ivf2: using Visual Studio bundled vcpkg (${_ivf2_vs_fallback})")
    return()
endif()

message(STATUS "ivf2: no vcpkg toolchain found - dependencies must be discoverable via CMAKE_PREFIX_PATH. "
               "Set VCPKG_ROOT or pass -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake.")
