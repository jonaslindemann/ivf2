#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <algorithm>

namespace ivf {

// ---- Range mapping -------------------------------------------------------

/** Re-maps a value from [inMin,inMax] to [outMin,outMax]. */
inline float map(float value, float inMin, float inMax, float outMin, float outMax) noexcept
{
    if (inMax == inMin)
        return outMin;
    return outMin + (value - inMin) / (inMax - inMin) * (outMax - outMin);
}

/** Clamps value to [lo, hi]. */
inline float constrain(float value, float lo, float hi) noexcept
{
    return std::clamp(value, lo, hi);
}

/** Oscillates between 0 and length, then back — like a triangle wave. */
inline float pingpong(float t, float length) noexcept
{
    float cycle = std::fmod(t, 2.0f * length);
    return cycle > length ? 2.0f * length - cycle : cycle;
}

// ---- Easing (normalized t in [0,1]) -------------------------------------

inline float easeInQuad(float t) noexcept    { return t * t; }
inline float easeOutQuad(float t) noexcept   { return t * (2.0f - t); }
inline float easeInOutQuad(float t) noexcept { return t < 0.5f ? 2.0f*t*t : -1.0f+(4.0f-2.0f*t)*t; }

inline float easeInCubic(float t) noexcept    { return t * t * t; }
inline float easeOutCubic(float t) noexcept   { float f=t-1.0f; return f*f*f+1.0f; }
inline float easeInOutCubic(float t) noexcept { return t<0.5f ? 4.0f*t*t*t : (t-1.0f)*(2.0f*t-2.0f)*(2.0f*t-2.0f)+1.0f; }

inline float easeInSine(float t) noexcept    { return 1.0f - std::cos(t * 1.5707963f); }
inline float easeOutSine(float t) noexcept   { return std::sin(t * 1.5707963f); }
inline float easeInOutSine(float t) noexcept { return 0.5f*(1.0f - std::cos(3.1415927f*t)); }

inline float easeOutElastic(float t) noexcept
{
    constexpr float c4 = 2.0943951f; // (2*pi)/3
    if (t == 0.0f || t == 1.0f) return t;
    return std::pow(2.0f, -10.0f*t) * std::sin((t*10.0f - 0.75f)*c4) + 1.0f;
}

inline float easeOutBounce(float t) noexcept
{
    constexpr float n1 = 7.5625f, d1 = 2.75f;
    if (t < 1.0f/d1)       return n1*t*t;
    else if (t < 2.0f/d1)  { t -= 1.5f/d1;   return n1*t*t + 0.75f; }
    else if (t < 2.5f/d1)  { t -= 2.25f/d1;  return n1*t*t + 0.9375f; }
    else                   { t -= 2.625f/d1; return n1*t*t + 0.984375f; }
}

inline float easeOutBack(float t) noexcept
{
    constexpr float c1 = 1.70158f, c3 = c1 + 1.0f;
    return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
}

// ---- Color conversions ---------------------------------------------------

/** HSV (h in [0,360], s and v in [0,1]) → RGB in [0,1]. */
inline glm::vec3 hsvToRgb(float h, float s, float v) noexcept
{
    if (s == 0.0f) return glm::vec3(v);
    h = std::fmod(h, 360.0f);
    if (h < 0.0f) h += 360.0f;
    float hi = std::floor(h / 60.0f);
    float f  = h / 60.0f - hi;
    float p  = v * (1.0f - s);
    float q  = v * (1.0f - s * f);
    float t2 = v * (1.0f - s * (1.0f - f));
    switch (static_cast<int>(hi) % 6) {
        case 0: return {v, t2, p};
        case 1: return {q, v, p};
        case 2: return {p, v, t2};
        case 3: return {p, q, v};
        case 4: return {t2, p, v};
        default: return {v, p, q};
    }
}

/** RGB in [0,1] → HSV (h in [0,360], s and v in [0,1]). */
inline glm::vec3 rgbToHsv(glm::vec3 c) noexcept
{
    float maxC = std::max({c.r, c.g, c.b});
    float minC = std::min({c.r, c.g, c.b});
    float delta = maxC - minC;
    float h = 0.0f, s = 0.0f, v = maxC;
    if (delta > 0.0f) {
        s = delta / maxC;
        if      (c.r >= maxC) h = 60.0f * std::fmod((c.g - c.b) / delta, 6.0f);
        else if (c.g >= maxC) h = 60.0f * ((c.b - c.r) / delta + 2.0f);
        else                  h = 60.0f * ((c.r - c.g) / delta + 4.0f);
        if (h < 0.0f) h += 360.0f;
    }
    return {h, s, v};
}

/**
 * Create a color from a packed hex value.
 * Example: colorFromHex(0xFF4488FF) → vec4(1, 0.267, 0.533, 1)
 * Byte order: RRGGBBAA.
 */
inline glm::vec4 colorFromHex(uint32_t hex) noexcept
{
    return {
        ((hex >> 24) & 0xFF) / 255.0f,
        ((hex >> 16) & 0xFF) / 255.0f,
        ((hex >>  8) & 0xFF) / 255.0f,
        ( hex        & 0xFF) / 255.0f
    };
}

// ---- CPU Perlin noise (1D / 2D / 3D) ------------------------------------

namespace detail {

inline float fade(float t) noexcept { return t*t*t*(t*(t*6.0f-15.0f)+10.0f); }
inline float grad1(int hash, float x) noexcept { return (hash & 1) ? x : -x; }
inline float grad2(int hash, float x, float y) noexcept
{
    return ((hash&1) ? x : -x) + ((hash&2) ? y : -y);
}
inline float grad3(int hash, float x, float y, float z) noexcept
{
    int h = hash & 15;
    float u = h<8 ? x : y;
    float v = h<4 ? y : (h==12||h==14 ? x : z);
    return ((h&1) ? -u : u) + ((h&2) ? -v : v);
}

// Permutation table
inline const int* perm() noexcept
{
    static const int p[512] = {
        151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,
        142,8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,
        203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,
        175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,
        230,220,105,92,41,55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,
        209,76,132,187,208,89,18,169,200,196,135,130,116,188,159,86,164,100,109,
        198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,
        212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,
        2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,39,253,19,98,108,110,
        79,113,224,232,178,185,112,104,218,246,97,228,251,34,242,193,238,210,144,
        12,191,179,162,241,81,51,145,235,249,14,239,107,49,192,214,31,181,199,
        106,157,184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,
        114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
        // repeat
        151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,
        142,8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,
        203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,
        175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,
        230,220,105,92,41,55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,
        209,76,132,187,208,89,18,169,200,196,135,130,116,188,159,86,164,100,109,
        198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,
        212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,
        2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,39,253,19,98,108,110,
        79,113,224,232,178,185,112,104,218,246,97,228,251,34,242,193,238,210,144,
        12,191,179,162,241,81,51,145,235,249,14,239,107,49,192,214,31,181,199,
        106,157,184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,
        114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
    };
    return p;
}

} // namespace detail

/** 1D Perlin noise, returns value roughly in [-1, 1]. */
inline float noise1d(float x) noexcept
{
    const int* p = detail::perm();
    int xi = static_cast<int>(std::floor(x)) & 255;
    float xf = x - std::floor(x);
    float u = detail::fade(xf);
    float a = detail::grad1(p[xi],   xf);
    float b = detail::grad1(p[xi+1], xf - 1.0f);
    return glm::mix(a, b, u);
}

/** 2D Perlin noise, returns value roughly in [-1, 1]. */
inline float noise2d(float x, float y) noexcept
{
    const int* p = detail::perm();
    int xi = static_cast<int>(std::floor(x)) & 255;
    int yi = static_cast<int>(std::floor(y)) & 255;
    float xf = x - std::floor(x);
    float yf = y - std::floor(y);
    float u = detail::fade(xf), v = detail::fade(yf);
    int aa = p[p[xi  ] + yi],   ab = p[p[xi  ] + yi+1];
    int ba = p[p[xi+1] + yi],   bb = p[p[xi+1] + yi+1];
    return glm::mix(
        glm::mix(detail::grad2(aa, xf,   yf),   detail::grad2(ba, xf-1.0f, yf),   u),
        glm::mix(detail::grad2(ab, xf,   yf-1.0f), detail::grad2(bb, xf-1.0f, yf-1.0f), u),
        v);
}

/** 3D Perlin noise, returns value roughly in [-1, 1]. */
inline float noise3d(float x, float y, float z) noexcept
{
    const int* p = detail::perm();
    int xi = static_cast<int>(std::floor(x)) & 255;
    int yi = static_cast<int>(std::floor(y)) & 255;
    int zi = static_cast<int>(std::floor(z)) & 255;
    float xf = x - std::floor(x);
    float yf = y - std::floor(y);
    float zf = z - std::floor(z);
    float u = detail::fade(xf), v = detail::fade(yf), w = detail::fade(zf);
    int aaa = p[p[p[xi  ]+yi  ]+zi  ], aab = p[p[p[xi  ]+yi  ]+zi+1];
    int aba = p[p[p[xi  ]+yi+1]+zi  ], abb = p[p[p[xi  ]+yi+1]+zi+1];
    int baa = p[p[p[xi+1]+yi  ]+zi  ], bab = p[p[p[xi+1]+yi  ]+zi+1];
    int bba = p[p[p[xi+1]+yi+1]+zi  ], bbb = p[p[p[xi+1]+yi+1]+zi+1];
    return glm::mix(
        glm::mix(glm::mix(detail::grad3(aaa,xf,  yf,  zf),   detail::grad3(baa,xf-1,yf,  zf),   u),
                 glm::mix(detail::grad3(aba,xf,  yf-1,zf),   detail::grad3(bba,xf-1,yf-1,zf),   u), v),
        glm::mix(glm::mix(detail::grad3(aab,xf,  yf,  zf-1), detail::grad3(bab,xf-1,yf,  zf-1), u),
                 glm::mix(detail::grad3(abb,xf,  yf-1,zf-1), detail::grad3(bbb,xf-1,yf-1,zf-1), u), v),
        w);
}

/** Fractional Brownian Motion — sum of noise at increasing frequencies. */
inline float fbm(float x, float y, int octaves = 4, float lacunarity = 2.0f, float gain = 0.5f) noexcept
{
    float value = 0.0f, amplitude = 0.5f, frequency = 1.0f;
    for (int i = 0; i < octaves; ++i) {
        value += amplitude * noise2d(x * frequency, y * frequency);
        amplitude *= gain;
        frequency *= lacunarity;
    }
    return value;
}

// ---- Curl noise (divergence-free vector fields) --------------------------
//
// 2D curl noise: curl of a scalar potential sampled from 3D Perlin noise.
// The result is divergence-free, making it ideal for fluid-like particle flow.
// Parameter t animates the field over time.
//
inline glm::vec2 curlNoise2D(float x, float y, float t = 0.0f, float eps = 0.01f) noexcept
{
    // φ(x,y,t) = noise3d(x,y,t)
    // curl_x = ∂φ/∂y,  curl_y = -∂φ/∂x
    float dPhi_dy = (noise3d(x,       y + eps, t) - noise3d(x,       y - eps, t)) / (2.0f * eps);
    float dPhi_dx = (noise3d(x + eps, y,       t) - noise3d(x - eps, y,       t)) / (2.0f * eps);
    return glm::vec2(dPhi_dy, -dPhi_dx);
}

// 3D curl noise: curl of a vector potential (Ax, Ay, Az) built from three
// independently decorrelated Perlin fields. Each field occupies its own
// region of noise space (large coordinate offset) and evolves with t.
//
// curl.x = ∂Az/∂y - ∂Ay/∂z
// curl.y = ∂Ax/∂z - ∂Az/∂x
// curl.z = ∂Ay/∂x - ∂Ax/∂y
//
// Ax(x,y,z) = noise3d( x,          y,          z + t        )
// Ay(x,y,z) = noise3d( x + kOff1,  y + kOff1,  z + kOff1+t  )
// Az(x,y,z) = noise3d( x + kOff2,  y + kOff2,  z + kOff2+t  )
//
inline glm::vec3 curlNoise3D(float x, float y, float z, float t = 0.0f, float eps = 0.01f) noexcept
{
    constexpr float kOff1 = 100.0f, kOff2 = 200.0f;

    // Ax partial derivatives
    float dAx_dy = (noise3d(x,        y + eps,  z + t)        - noise3d(x,        y - eps,  z + t))        / (2.0f * eps);
    float dAx_dz = (noise3d(x,        y,        z + eps + t)  - noise3d(x,        y,        z - eps + t))  / (2.0f * eps);

    // Ay partial derivatives
    float dAy_dx = (noise3d(x + eps + kOff1, y + kOff1, z + kOff1 + t) - noise3d(x - eps + kOff1, y + kOff1, z + kOff1 + t)) / (2.0f * eps);
    float dAy_dz = (noise3d(x + kOff1, y + kOff1, z + eps + kOff1 + t) - noise3d(x + kOff1, y + kOff1, z - eps + kOff1 + t)) / (2.0f * eps);

    // Az partial derivatives
    float dAz_dx = (noise3d(x + eps + kOff2, y + kOff2, z + kOff2 + t) - noise3d(x - eps + kOff2, y + kOff2, z + kOff2 + t)) / (2.0f * eps);
    float dAz_dy = (noise3d(x + kOff2, y + eps + kOff2, z + kOff2 + t) - noise3d(x + kOff2, y - eps + kOff2, z + kOff2 + t)) / (2.0f * eps);

    return glm::vec3(dAz_dy - dAy_dz, dAx_dz - dAz_dx, dAy_dx - dAx_dy);
}

} // namespace ivf
