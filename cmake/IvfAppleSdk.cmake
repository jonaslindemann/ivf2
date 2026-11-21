# IvfAppleSdk.cmake
#
# Included before project(). On macOS, verify the selected SDK ships a working
# std::format. Two known-broken cases:
#   1. Older SDKs define _LIBCPP_HAS_NO_INCOMPLETE_FORMAT, disabling <format>.
#   2. Xcode MacOSX14 SDKs have a std::format compilation bug with Apple Clang 15+.
# In either case fall back to a CommandLineTools SDK with a correct implementation.

if(NOT CMAKE_HOST_APPLE OR CMAKE_OSX_SYSROOT)
    return()
endif()

execute_process(
    COMMAND xcrun --sdk macosx --show-sdk-path
    OUTPUT_VARIABLE _ivf2_sdk
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)

function(_ivf2_sdk_has_format sdk out)
    set(${out} TRUE PARENT_SCOPE)
    if(EXISTS "${sdk}/usr/include/c++/v1/__config")
        file(READ "${sdk}/usr/include/c++/v1/__config" _cfg)
        string(FIND "${_cfg}" "_LIBCPP_HAS_NO_INCOMPLETE_FORMAT" _pos)
        if(NOT _pos EQUAL -1)
            set(${out} FALSE PARENT_SCOPE)
        endif()
    endif()
endfunction()

set(_ivf2_format_ok TRUE)
if(_ivf2_sdk)
    _ivf2_sdk_has_format("${_ivf2_sdk}" _ivf2_format_ok)
    string(FIND "${_ivf2_sdk}" "MacOSX14" _ivf2_sdk14)
    if(NOT _ivf2_sdk14 EQUAL -1)
        set(_ivf2_format_ok FALSE)
    endif()
endif()

if(NOT _ivf2_format_ok)
    message(STATUS "ivf2: SDK ${_ivf2_sdk} has a broken std::format - searching CommandLineTools SDKs")
    file(GLOB _ivf2_clt_sdks "/Library/Developer/CommandLineTools/SDKs/MacOSX1[5-9]*.sdk"
                             "/Library/Developer/CommandLineTools/SDKs/MacOSX2*.sdk")
    list(SORT _ivf2_clt_sdks COMPARE NATURAL ORDER DESCENDING)
    foreach(_cand IN LISTS _ivf2_clt_sdks)
        _ivf2_sdk_has_format("${_cand}" _ok)
        if(_ok AND EXISTS "${_cand}/usr/include/c++/v1/__config")
            set(CMAKE_OSX_SYSROOT "${_cand}" CACHE PATH "macOS SDK with std::format support" FORCE)
            message(STATUS "ivf2: using SDK ${_cand}")
            break()
        endif()
    endforeach()
endif()
