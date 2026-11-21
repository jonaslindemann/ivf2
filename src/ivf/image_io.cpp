/**
 * @file stb_impl.cpp
 * @brief Single translation unit holding the stb_image / stb_image_write implementations.
 *
 * Lives in the ivf core library because ivf::Texture and ivf::Cubemap use stb_image;
 * ivfui (screenshots, frame export) links ivf and reuses these symbols.
 * This file is compiled without the precompiled header (see src/ivf/CMakeLists.txt)
 * so the *_IMPLEMENTATION defines are not swallowed by an earlier include.
 */

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
