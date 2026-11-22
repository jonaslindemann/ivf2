#pragma once
/**
 * @file version.h
 * @brief ivf2 version information (from the CMake project version, see <ivf/config.h>).
 */

#include <ivf/config.h>

namespace ivf {
inline constexpr int version_major = IVF2_VERSION_MAJOR;
inline constexpr int version_minor = IVF2_VERSION_MINOR;
inline constexpr int version_patch = IVF2_VERSION_PATCH;
inline constexpr const char *version_prerelease = IVF2_VERSION_PRERELEASE; ///< "" for a final release.
inline constexpr const char *version_name = "ivf2";
inline constexpr const char *version_long_name = "Interactive Visualisation Framework 2";
inline constexpr const char *version_string = IVF2_VERSION_STRING; ///< e.g. "2.0.0-alpha".
inline constexpr const char *version_date = __DATE__;
} // namespace ivf
