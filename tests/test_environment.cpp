// Image-based lighting inputs without OpenGL: the procedural sky image and HDR file I/O.
// (The GPU precomputation is covered by the ibl1 visual test.)

#include "test_main.h"

#include <ivf/environment_map.h>
#include <ivf/image_io.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <string>

using namespace ivf;

namespace {

// Direction of pixel (x, y) of an equirectangular image, as the environment shader maps it
glm::vec3 pixelDirection(int x, int y, int width, int height)
{
    const float pi = glm::pi<float>();
    const float theta = (float(y) + 0.5f) / float(height) * pi;
    const float phi = ((float(x) + 0.5f) / float(width) - 0.5f) * 2.0f * pi;
    return {std::sin(theta) * std::cos(phi), std::cos(theta), std::sin(theta) * std::sin(phi)};
}

float luminance(const float *p)
{
    return 0.2126f * p[0] + 0.7152f * p[1] + 0.0722f * p[2];
}

} // namespace

IVF_TEST(sky_is_brightest_towards_the_sun)
{
    SkySettings settings;
    settings.width = 256;
    settings.sunDirection = glm::normalize(glm::vec3(-0.3f, 0.6f, 0.7f));
    const auto image = EnvironmentMap::skyImage(settings);
    const int width = settings.width, height = width / 2;
    IVF_CHECK(image.size() == std::size_t(width) * height * 3);

    // The brightest pixel lies in the sun's direction
    int best = 0;
    for (int i = 1; i < width * height; ++i)
        if (luminance(&image[std::size_t(i) * 3]) > luminance(&image[std::size_t(best) * 3]))
            best = i;
    const glm::vec3 d = pixelDirection(best % width, best / width, width, height);
    IVF_CHECK(glm::dot(d, settings.sunDirection) > 0.995f);

    // HDR: the sun is far brighter than white, the sky above the ground
    IVF_CHECK(luminance(&image[std::size_t(best) * 3]) > 10.0f);
    const float top = luminance(&image[std::size_t(width / 4) * 3]);                         // up
    const float bottom = luminance(&image[(std::size_t(height - 1) * width + width / 4) * 3]); // down
    IVF_CHECK(top > bottom);
}

IVF_TEST(hdr_files_round_trip)
{
    SkySettings settings;
    settings.width = 64;
    const auto image = EnvironmentMap::skyImage(settings);
    const std::string path = (std::filesystem::temp_directory_path() / "ivf2_test_sky.hdr").string();
    IVF_CHECK(writeHdr(path.c_str(), 64, 32, 3, image.data()));

    int w = 0, h = 0, channels = 0;
    float *data = loadImageF(path.c_str(), &w, &h, &channels, 3);
    IVF_CHECK(data != nullptr);
    if (data != nullptr)
    {
        IVF_CHECK(w == 64 && h == 32);
        // RGBE keeps about 1% precision, also far above 1
        for (int i = 0; i < w * h * 3; i += 97)
            IVF_CHECK_NEAR(data[i], image[std::size_t(i)], 0.02 * image[std::size_t(i)] + 1e-3);
        freeImage(data);
    }
    std::remove(path.c_str());
}

IVF_TEST(flip_setting_can_be_read_back)
{
    IVF_CHECK(!imageFlipVerticallyOnLoad());
    setImageFlipVerticallyOnLoad(true);
    IVF_CHECK(imageFlipVerticallyOnLoad());
    setImageFlipVerticallyOnLoad(false);
    IVF_CHECK(!imageFlipVerticallyOnLoad());
}

IVF_TEST_MAIN()
