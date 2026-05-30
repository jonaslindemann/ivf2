#pragma once

#include <ivf/math_utils.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

namespace ivf {

/**
 * @brief Named color palettes and color harmony generators for creative coding.
 *
 * All colors are returned as glm::vec4 (RGBA, components in [0,1]).
 * Use Palette::get() to retrieve a named palette, or the harmony generators
 * (complementary, analogous, triadic) to derive palettes from a seed color.
 */
class Palette {
public:
    // ---- Named palettes -------------------------------------------------

    static std::vector<glm::vec4> solarized()
    {
        return {
            colorFromHex(0x002B36FF), colorFromHex(0x073642FF), colorFromHex(0x586E75FF),
            colorFromHex(0x657B83FF), colorFromHex(0x839496FF), colorFromHex(0x93A1A1FF),
            colorFromHex(0xEEE8D5FF), colorFromHex(0xFDF6E3FF), colorFromHex(0xB58900FF),
            colorFromHex(0xCB4B16FF), colorFromHex(0xDC322FFF), colorFromHex(0xD33682FF),
            colorFromHex(0x6C71C4FF), colorFromHex(0x268BD2FF), colorFromHex(0x2AA198FF),
            colorFromHex(0x859900FF)
        };
    }

    static std::vector<glm::vec4> nord()
    {
        return {
            colorFromHex(0x2E3440FF), colorFromHex(0x3B4252FF), colorFromHex(0x434C5EFF),
            colorFromHex(0x4C566AFF), colorFromHex(0xD8DEE9FF), colorFromHex(0xE5E9F0FF),
            colorFromHex(0xECEFF4FF), colorFromHex(0x8FBCBBFF), colorFromHex(0x88C0D0FF),
            colorFromHex(0x81A1C1FF), colorFromHex(0x5E81ACFF), colorFromHex(0xBF616AFF),
            colorFromHex(0xD08770FF), colorFromHex(0xEBCB8BFF), colorFromHex(0xA3BE8CFF),
            colorFromHex(0xB48EADFF)
        };
    }

    static std::vector<glm::vec4> dracula()
    {
        return {
            colorFromHex(0x282A36FF), colorFromHex(0x44475AFF), colorFromHex(0xF8F8F2FF),
            colorFromHex(0x6272A4FF), colorFromHex(0x8BE9FDFF), colorFromHex(0x50FA7BFF),
            colorFromHex(0xFFB86CFF), colorFromHex(0xFF79C6FF), colorFromHex(0xBD93F9FF),
            colorFromHex(0xFF5555FF), colorFromHex(0xF1FA8CFF)
        };
    }

    static std::vector<glm::vec4> monokai()
    {
        return {
            colorFromHex(0x272822FF), colorFromHex(0x75715EFF), colorFromHex(0xF8F8F2FF),
            colorFromHex(0x66D9E8FF), colorFromHex(0xA6E22EFF), colorFromHex(0xF92672FF),
            colorFromHex(0xFD971FFF), colorFromHex(0xAE81FFFF), colorFromHex(0xE6DB74FF)
        };
    }

    static std::vector<glm::vec4> sunset()
    {
        return {
            colorFromHex(0x0D0221FF), colorFromHex(0x0A1045FF), colorFromHex(0x230470FF),
            colorFromHex(0x6B0AC9FF), colorFromHex(0xC400C4FF), colorFromHex(0xFF3EA5FF),
            colorFromHex(0xFF7676FF), colorFromHex(0xFFB347FF), colorFromHex(0xFFE066FF)
        };
    }

    static std::vector<glm::vec4> ocean()
    {
        return {
            colorFromHex(0x000814FF), colorFromHex(0x001D3DFF), colorFromHex(0x003566FF),
            colorFromHex(0x006494FF), colorFromHex(0x0096C7FF), colorFromHex(0x00B4D8FF),
            colorFromHex(0x48CAE4FF), colorFromHex(0x90E0EFFF), colorFromHex(0xADE8F4FF),
            colorFromHex(0xCAF0F8FF)
        };
    }

    static std::vector<glm::vec4> forest()
    {
        return {
            colorFromHex(0x081C15FF), colorFromHex(0x1B4332FF), colorFromHex(0x2D6A4FFF),
            colorFromHex(0x40916CFF), colorFromHex(0x52B788FF), colorFromHex(0x74C69DFF),
            colorFromHex(0x95D5B2FF), colorFromHex(0xB7E4C7FF), colorFromHex(0xD8F3DCFF)
        };
    }

    /** Return a named palette by string key. Fallback: returns solarized(). */
    static std::vector<glm::vec4> get(const std::string& name)
    {
        if (name == "nord")     return nord();
        if (name == "dracula")  return dracula();
        if (name == "monokai")  return monokai();
        if (name == "sunset")   return sunset();
        if (name == "ocean")    return ocean();
        if (name == "forest")   return forest();
        return solarized();
    }

    // ---- Color harmony generators ---------------------------------------

    /**
     * Complementary pair: seed color + its opposite on the color wheel.
     * @param rgb Seed color in [0,1].
     */
    static std::vector<glm::vec4> complementary(glm::vec3 rgb)
    {
        glm::vec3 hsv = rgbToHsv(rgb);
        glm::vec3 comp = hsvToRgb(std::fmod(hsv.x + 180.0f, 360.0f), hsv.y, hsv.z);
        return { glm::vec4(rgb, 1), glm::vec4(comp, 1) };
    }

    /**
     * Analogous triad: seed ± 30° on the color wheel.
     * @param rgb Seed color in [0,1].
     */
    static std::vector<glm::vec4> analogous(glm::vec3 rgb)
    {
        glm::vec3 hsv = rgbToHsv(rgb);
        return {
            glm::vec4(hsvToRgb(std::fmod(hsv.x - 30.0f + 360.0f, 360.0f), hsv.y, hsv.z), 1),
            glm::vec4(rgb, 1),
            glm::vec4(hsvToRgb(std::fmod(hsv.x + 30.0f, 360.0f), hsv.y, hsv.z), 1)
        };
    }

    /**
     * Triadic: seed + 120° + 240° on the color wheel.
     * @param rgb Seed color in [0,1].
     */
    static std::vector<glm::vec4> triadic(glm::vec3 rgb)
    {
        glm::vec3 hsv = rgbToHsv(rgb);
        return {
            glm::vec4(rgb, 1),
            glm::vec4(hsvToRgb(std::fmod(hsv.x + 120.0f, 360.0f), hsv.y, hsv.z), 1),
            glm::vec4(hsvToRgb(std::fmod(hsv.x + 240.0f, 360.0f), hsv.y, hsv.z), 1)
        };
    }

    /**
     * Generate N evenly-spaced hues at fixed saturation and value.
     * Great for distinguishing many objects.
     */
    static std::vector<glm::vec4> rainbow(int count, float saturation = 0.8f, float value = 0.9f)
    {
        std::vector<glm::vec4> result;
        result.reserve(count);
        for (int i = 0; i < count; ++i)
            result.emplace_back(glm::vec4(hsvToRgb(360.0f * i / count, saturation, value), 1.0f));
        return result;
    }

    /**
     * Lerp between two colors in HSV space (maintains perceptual smoothness).
     */
    static glm::vec4 lerpHSV(glm::vec4 a, glm::vec4 b, float t)
    {
        glm::vec3 ha = rgbToHsv(glm::vec3(a));
        glm::vec3 hb = rgbToHsv(glm::vec3(b));
        // Handle hue wrap-around
        float dh = hb.x - ha.x;
        if (dh > 180.0f)  dh -= 360.0f;
        if (dh < -180.0f) dh += 360.0f;
        glm::vec3 rgb = hsvToRgb(ha.x + dh * t, ha.y + (hb.y - ha.y) * t, ha.z + (hb.z - ha.z) * t);
        return { rgb, a.a + (b.a - a.a) * t };
    }
};

} // namespace ivf
