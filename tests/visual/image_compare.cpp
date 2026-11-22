// image_compare - compare a rendered image with a reference image.
//
//   image_compare <reference.png> <actual.png> [--diff diff.png]
//                 [--threshold N] [--max-fraction F]
//
// A pixel differs when one of its RGB channels differs by more than N (0-255,
// default 24). The images match when they have the same size and at most the
// fraction F (default 0.01) of their pixels differ. This tolerates the small
// rasterisation and precision differences between driver versions while still
// catching missing objects, wrong colours or broken effects.
//
// The diff image shows the reference dimmed to grey, with differing pixels in red.
// Exit code: 0 match, 1 mismatch, 2 usage or file error.

#include <ivf/image_io.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

struct Image {
    int width{0};
    int height{0};
    std::vector<unsigned char> rgb;
};

bool load(const std::string &filename, Image &image)
{
    int channels = 0;
    unsigned char *data = ivf::loadImage(filename.c_str(), &image.width, &image.height, &channels, 3);
    if (!data)
    {
        std::fprintf(stderr, "image_compare: cannot read %s (%s)\n", filename.c_str(), ivf::imageFailureReason());
        return false;
    }
    image.rgb.assign(data, data + size_t(image.width) * size_t(image.height) * 3);
    ivf::freeImage(data);
    return true;
}

int usage()
{
    std::fprintf(stderr, "usage: image_compare <reference.png> <actual.png> [--diff diff.png] "
                         "[--threshold N] [--max-fraction F]\n");
    return 2;
}

} // namespace

int main(int argc, char **argv)
{
    std::vector<std::string> files;
    std::string diffFile;
    int threshold = 24;
    double maxFraction = 0.01;

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        const bool hasValue = i + 1 < argc;
        if (arg == "--diff" && hasValue)
            diffFile = argv[++i];
        else if (arg == "--threshold" && hasValue)
            threshold = std::atoi(argv[++i]);
        else if (arg == "--max-fraction" && hasValue)
            maxFraction = std::atof(argv[++i]);
        else if (arg.starts_with("--"))
            return usage();
        else
            files.push_back(arg);
    }
    if (files.size() != 2)
        return usage();

    Image reference, actual;
    if (!load(files[0], reference) || !load(files[1], actual))
        return 2;

    if (reference.width != actual.width || reference.height != actual.height)
    {
        std::printf("size differs: reference %dx%d, actual %dx%d\n", reference.width, reference.height,
                    actual.width, actual.height);
        return 1;
    }

    const size_t pixelCount = size_t(reference.width) * size_t(reference.height);
    std::vector<unsigned char> diff(pixelCount * 3);
    size_t differing = 0;
    double totalError = 0.0;
    int maxError = 0;

    for (size_t p = 0; p < pixelCount; ++p)
    {
        const unsigned char *r = &reference.rgb[p * 3];
        const unsigned char *a = &actual.rgb[p * 3];
        int pixelError = 0;
        for (int c = 0; c < 3; ++c)
        {
            const int e = std::abs(int(r[c]) - int(a[c]));
            totalError += e;
            pixelError = std::max(pixelError, e);
        }
        maxError = std::max(maxError, pixelError);

        unsigned char *d = &diff[p * 3];
        if (pixelError > threshold)
        {
            ++differing;
            d[0] = 255;
            d[1] = 0;
            d[2] = 0;
        }
        else
        {
            const auto grey = static_cast<unsigned char>((int(r[0]) + int(r[1]) + int(r[2])) / 9);
            d[0] = d[1] = d[2] = grey;
        }
    }

    const double fraction = pixelCount > 0 ? double(differing) / double(pixelCount) : 0.0;
    const double meanError = pixelCount > 0 ? totalError / double(pixelCount * 3) : 0.0;
    const bool match = fraction <= maxFraction;

    std::printf("%s: %zu of %zu pixels differ by more than %d (%.3f%%, limit %.3f%%), "
                "mean error %.2f, max error %d\n",
                match ? "match" : "MISMATCH", differing, pixelCount, threshold, fraction * 100.0,
                maxFraction * 100.0, meanError, maxError);

    if (!diffFile.empty() && !ivf::writePng(diffFile.c_str(), reference.width, reference.height, 3, diff.data(),
                                            reference.width * 3))
        std::fprintf(stderr, "image_compare: cannot write %s\n", diffFile.c_str());

    return match ? 0 : 1;
}
