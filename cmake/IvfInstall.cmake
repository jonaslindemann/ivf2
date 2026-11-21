# IvfInstall.cmake - install rules and CMake package for ivf2.
#
# After `cmake --install <build> --prefix <dir>` downstream projects use:
#
#   find_package(ivf2 CONFIG REQUIRED)
#   add_executable(app main.cpp)
#   target_link_libraries(app PRIVATE ivf2::ivf2)   # or ivf2::ivfmusic
#   ivf2_copy_runtime_files(app)                     # shaders/ + fonts/ next to app

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(IVF2_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/ivf2")
set(IVF2_INSTALL_DATADIR "${CMAKE_INSTALL_DATADIR}/ivf2")

get_property(_ivf2_targets GLOBAL PROPERTY IVF2_EXPORTED_TARGETS)

install(TARGETS ${_ivf2_targets}
    EXPORT ivf2Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${IVF2_INSTALL_INCLUDEDIR})

# Public headers, including the bundled glm/glad/KHR, below include/ivf2/.
foreach(_dir ivf ivfui ivfmath ivfmusic generator glad KHR glm)
    if(_dir STREQUAL "ivfmusic" AND NOT IVF2_BUILD_MUSIC)
        continue()
    endif()
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/${_dir}"
        DESTINATION ${IVF2_INSTALL_INCLUDEDIR}
        FILES_MATCHING
            PATTERN "*.h"
            PATTERN "*.hpp"
            PATTERN "*.inl")
endforeach()

# Generated build configuration (feature switches, version).
install(FILES "${IVF2_GENERATED_INCLUDE_DIR}/ivf/config.h" DESTINATION ${IVF2_INSTALL_INCLUDEDIR}/ivf)

# Runtime data needed by applications.
install(DIRECTORY "${IVF2_RUNTIME_DATA_DIR}/shaders" "${IVF2_RUNTIME_DATA_DIR}/fonts"
    DESTINATION ${IVF2_INSTALL_DATADIR})

install(EXPORT ivf2Targets
    NAMESPACE ivf2::
    DESTINATION ${IVF2_INSTALL_CMAKEDIR})

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/ivf2Config.cmake.in"
    "${PROJECT_BINARY_DIR}/ivf2Config.cmake"
    INSTALL_DESTINATION ${IVF2_INSTALL_CMAKEDIR}
    PATH_VARS IVF2_INSTALL_DATADIR)

write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/ivf2ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES
    "${PROJECT_BINARY_DIR}/ivf2Config.cmake"
    "${PROJECT_BINARY_DIR}/ivf2ConfigVersion.cmake"
    "${PROJECT_SOURCE_DIR}/cmake/ivf2Helpers.cmake"
    "${PROJECT_SOURCE_DIR}/cmake/ivfmusic.cmake"
    DESTINATION ${IVF2_INSTALL_CMAKEDIR})

install(FILES "${PROJECT_SOURCE_DIR}/LICENSE" DESTINATION "${CMAKE_INSTALL_DATADIR}/doc/ivf2")
