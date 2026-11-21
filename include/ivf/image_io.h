#pragma once
/**
 * @file image_io.h
 * @brief Image file reading and writing.
 *
 * Thin wrappers around a private copy of stb_image / stb_image_write. The stb
 * functions are compiled with internal linkage inside the ivf library, so they
 * never clash with other libraries that embed their own stb (for example a
 * statically linked assimp), and users of ivf do not need stb headers.
 */

namespace ivf {

/**
 * @brief Load an 8-bit image file (PNG, JPEG, BMP, TGA, ...).
 * @param filename Path to the image.
 * @param width Receives the width in pixels.
 * @param height Receives the height in pixels.
 * @param channels Receives the number of channels in the file (may be nullptr).
 * @param desiredChannels Force this many channels (1-4), or 0 to keep the file's.
 * @return Pixel data to be released with freeImage(), or nullptr on failure
 *         (see imageFailureReason()).
 */
[[nodiscard]] unsigned char *loadImage(const char *filename, int *width, int *height, int *channels,
                                       int desiredChannels = 0);

/**
 * @brief Load an image as 32-bit floats (HDR files keep their range).
 * @return Pixel data to be released with freeImage(), or nullptr on failure.
 */
[[nodiscard]] float *loadImageF(const char *filename, int *width, int *height, int *channels,
                                int desiredChannels = 0);

/** @brief Release pixel data returned by loadImage() or loadImageF(). */
void freeImage(void *data);

/** @brief Flip images vertically while loading (global setting, default off). */
void setImageFlipVerticallyOnLoad(bool flip);

/** @brief Description of the last loading error. */
const char *imageFailureReason();

/**
 * @brief Write pixel data to a PNG file.
 * @param strideBytes Bytes between rows; may be negative to write bottom-up data.
 * @return true on success.
 */
bool writePng(const char *filename, int width, int height, int channels, const void *data, int strideBytes);

} // namespace ivf
