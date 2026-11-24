#pragma once

/**
 * @file tone_mapping.h
 * @brief Tone mapping operators for the HDR output step.
 *
 * With render to texture, scenes are drawn into a floating-point (RGBA16F) image, so
 * bright lights and additive glows can exceed 1. The output step maps that range to the
 * display: scale by the exposure, apply a tone mapping operator, then a gamma curve
 * (see ivfui::GLFWSceneWindow::setToneMapping()).
 */

namespace ivf {

/// Curve that maps HDR colours (0 to infinity) to the display range (0 to 1).
enum class ToneMapping {
    None,     ///< No curve: values above 1 clip (exposure and gamma still apply).
    Reinhard, ///< c / (1 + c) per channel: soft, never clips, flattens contrast.
    Aces,     ///< ACES filmic fit (RRT + ODT, Stephen Hill): contrasty, saturated highlights.
    AgX       ///< AgX (Troy Sobotka, minimal fit by Benjamin Wrensch): bright colours desaturate towards white.
};

/// Number of ToneMapping values (for UI lists).
inline constexpr int ToneMappingCount = 4;

/// Display name of @p toneMapping.
[[nodiscard]] constexpr const char *toneMappingName(ToneMapping toneMapping)
{
    switch (toneMapping)
    {
    case ToneMapping::None:
        return "None";
    case ToneMapping::Reinhard:
        return "Reinhard";
    case ToneMapping::Aces:
        return "ACES";
    case ToneMapping::AgX:
        return "AgX";
    }
    return "Unknown";
}

} // namespace ivf
