# Package consumer test.
#
# A minimal, separate project that uses an *installed* ivf2 exactly like a
# downstream user would. CI builds it after `cmake --install` to make sure the
# exported targets, ivf2Config.cmake and the helper functions keep working.
#
#   cmake -S tests/package -B build/package \
#         -DCMAKE_PREFIX_PATH=<ivf2 install prefix> \
#         -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake \
#         -DVCPKG_MANIFEST_MODE=OFF -DVCPKG_INSTALLED_DIR=<ivf2 source>/vcpkg_installed
#   cmake --build build/package --config Release

cmake_minimum_required(VERSION 3.24)
project(ivf2_package_test LANGUAGES CXX)

find_package(ivf2 2.0 CONFIG REQUIRED)

add_executable(package_test main.cpp)
target_link_libraries(package_test PRIVATE ivf2::ivf2)
ivf2_copy_runtime_files(package_test)

if(TARGET ivf2::ivfvis)
    add_executable(package_test_vis main.cpp)
    target_link_libraries(package_test_vis PRIVATE ivf2::ivf2 ivf2::ivfvis)
endif()

if(TARGET ivf2::ivfmusic)
    add_executable(package_test_music main.cpp)
    target_link_libraries(package_test_music PRIVATE ivf2::ivfmusic)
endif()
