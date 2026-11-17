#include <ivf/mip_bloom_effect.h>

#include <ivf/post_shaders.h>
#include <ivf/shader_manager.h>

using namespace ivf;

namespace {

const std::string mip_bloom_frag_shader_source = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform int screenTextureLevels; // requests a mip chain of the input
uniform float glowThreshold;
uniform float glowIntensity;
uniform float glowSpread;
uniform float glowExposure;

// A NaN or infinite pixel spreads through the whole mip chain; drop it instead
vec3 sanitize(vec3 color)
{
    return (any(isnan(color)) || any(isinf(color))) ? vec3(0.0) : color;
}

// Bright part of a color, with a soft knee around the threshold
vec3 brightPart(vec3 color)
{
    float luma = max(color.r, max(color.g, color.b));
    float knee = glowThreshold * 0.5;
    float soft = clamp(luma - glowThreshold + knee, 0.0, 2.0 * knee);
    soft = soft * soft / (4.0 * knee + 1e-4);
    float contribution = max(soft, luma - glowThreshold) / max(luma, 1e-4);
    return color * contribution;
}

// Tent-filtered sample of a mip level, hides the blockiness of box-filtered mips
vec3 blurredLevel(float level)
{
    vec2 texel = 1.0 / vec2(textureSize(screenTexture, int(level)));
    vec3 sum = textureLod(screenTexture, TexCoords, level).rgb * 4.0;
    sum += textureLod(screenTexture, TexCoords + vec2(-texel.x, 0.0), level).rgb * 2.0;
    sum += textureLod(screenTexture, TexCoords + vec2( texel.x, 0.0), level).rgb * 2.0;
    sum += textureLod(screenTexture, TexCoords + vec2(0.0, -texel.y), level).rgb * 2.0;
    sum += textureLod(screenTexture, TexCoords + vec2(0.0,  texel.y), level).rgb * 2.0;
    sum += textureLod(screenTexture, TexCoords + vec2(-texel.x, -texel.y), level).rgb;
    sum += textureLod(screenTexture, TexCoords + vec2( texel.x, -texel.y), level).rgb;
    sum += textureLod(screenTexture, TexCoords + vec2(-texel.x,  texel.y), level).rgb;
    sum += textureLod(screenTexture, TexCoords + vec2( texel.x,  texel.y), level).rgb;
    return sanitize(sum / 16.0);
}

void main()
{
    vec4 base = textureLod(screenTexture, TexCoords, 0.0);
    base.rgb = sanitize(base.rgb);

    vec3 glow = vec3(0.0);
    float total = 0.0;
    float weight = 1.0;
    for (int level = 1; level <= 7; ++level) {
        if (level >= screenTextureLevels)
            break;
        glow += brightPart(blurredLevel(float(level))) * weight;
        total += weight;
        weight *= glowSpread;
    }
    glow /= max(total, 1e-4);

    vec3 color = (base.rgb + glow * glowIntensity) * glowExposure;

    // Soft shoulder: unchanged below 0.8, rolls off smoothly towards 1 above it
    // (same slope at the joint, so there is no visible kink)
    const float shoulder = 0.8;
    vec3 over = max(color - shoulder, 0.0);
    color = min(color, vec3(shoulder)) + (1.0 - shoulder) * (1.0 - exp(-over / (1.0 - shoulder)));

    FragColor = vec4(color, base.a);
})";

} // namespace

MipBloomEffect::MipBloomEffect()
{
    setName("Mip Bloom");
}

std::shared_ptr<MipBloomEffect> MipBloomEffect::create()
{
    return std::make_shared<MipBloomEffect>();
}

void MipBloomEffect::doLoad()
{
    m_program = smLoadProgramFromStrings(ivf::render_to_texture_vert_shader_source_330, mip_bloom_frag_shader_source,
                                         "mip_bloom", false);
}

void MipBloomEffect::doUpdateParams()
{
    m_program->uniformFloat("glowThreshold", m_threshold);
    m_program->uniformFloat("glowIntensity", m_intensity);
    m_program->uniformFloat("glowSpread", m_spread);
    m_program->uniformFloat("glowExposure", m_exposure);
}

void MipBloomEffect::setupProperties()
{
    addProperty("Threshold", &m_threshold, 0.0, 4.0, "Mip Bloom");
    addProperty("Intensity", &m_intensity, 0.0, 4.0, "Mip Bloom");
    addProperty("Spread", &m_spread, 0.0, 1.0, "Mip Bloom");
    addProperty("Exposure", &m_exposure, 0.0, 4.0, "Mip Bloom");
}

void MipBloomEffect::setThreshold(float threshold)
{
    m_threshold = threshold;
}

void MipBloomEffect::setIntensity(float intensity)
{
    m_intensity = intensity;
}

void MipBloomEffect::setSpread(float spread)
{
    m_spread = spread;
}

void MipBloomEffect::setExposure(float exposure)
{
    m_exposure = exposure;
}

float MipBloomEffect::threshold() const
{
    return m_threshold;
}

float MipBloomEffect::intensity() const
{
    return m_intensity;
}

float MipBloomEffect::spread() const
{
    return m_spread;
}

float MipBloomEffect::exposure() const
{
    return m_exposure;
}
