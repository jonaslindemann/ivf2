/**
 * @file image_io.cpp
 * @brief ivf image I/O, implemented with a private copy of stb_image / stb_image_write.
 *
 * STB_IMAGE_STATIC / STB_IMAGE_WRITE_STATIC give every stb function internal
 * linkage, so this copy cannot collide with stb symbols embedded in other
 * static libraries (assimp ships its own stb_image on Linux/macOS).
 * Compiled without the precompiled header (see src/ivf/CMakeLists.txt) so no
 * earlier stb include can bypass the defines below.
 */

#include <ivf/image_io.h>

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function" // stb functions ivf does not call
#endif

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace ivf {

unsigned char *loadImage(const char *filename, int *width, int *height, int *channels, int desiredChannels)
{
    int dummy = 0;
    return stbi_load(filename, width, height, channels ? channels : &dummy, desiredChannels);
}

unsigned char *loadImageFromMemory(const unsigned char *bytes, int size, int *width, int *height, int *channels,
                                   int desiredChannels)
{
    int dummy = 0;
    return stbi_load_from_memory(bytes, size, width, height, channels ? channels : &dummy, desiredChannels);
}

float *loadImageF(const char *filename, int *width, int *height, int *channels, int desiredChannels)
{
    int dummy = 0;
    return stbi_loadf(filename, width, height, channels ? channels : &dummy, desiredChannels);
}

void freeImage(void *data)
{
    stbi_image_free(data);
}

namespace {
bool g_flipVertically = false; // mirrors stb's global setting, which has no getter
}

void setImageFlipVerticallyOnLoad(bool flip)
{
    g_flipVertically = flip;
    stbi_set_flip_vertically_on_load(flip ? 1 : 0);
}

bool imageFlipVerticallyOnLoad()
{
    return g_flipVertically;
}

const char *imageFailureReason()
{
    return stbi_failure_reason();
}

bool writePng(const char *filename, int width, int height, int channels, const void *data, int strideBytes)
{
    return stbi_write_png(filename, width, height, channels, data, strideBytes) != 0;
}

bool writeHdr(const char *filename, int width, int height, int channels, const float *data)
{
    return stbi_write_hdr(filename, width, height, channels, data) != 0;
}

} // namespace ivf
