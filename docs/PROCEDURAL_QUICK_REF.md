# Procedural Textures Quick Reference

## Include
```cpp
#include <ivf/procedural.h>
```

## Checkerboard Texture
```cpp
auto checker = CheckerboardTexture::create();
checker->setSize(512, 512);
checker->setCheckerSize(32);
checker->setColors(
    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),  // White
    glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)   // Black
);
checker->regenerate();
```

**Parameters:**
- `setCheckerSize(int size)` - Size of each checker square (default: 32)
- `setColors(color1, color2)` - Two alternating colors

## Perlin Noise Texture
```cpp
auto noise = PerlinNoiseTexture::create();
noise->setSize(512, 512);
noise->setScale(4.0f);
noise->setOctaves(4);
noise->setPersistence(0.5f);
noise->setLacunarity(2.0f);
noise->setColorRange(
    glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),  // Low values
    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)   // High values
);
noise->regenerate();
```

**Parameters:**
- `setScale(float)` - Noise scale/zoom (default: 4.0)
- `setOctaves(int)` - Detail levels (default: 4, range: 1-8)
- `setPersistence(float)` - Amplitude falloff (default: 0.5, range: 0.1-0.9)
- `setLacunarity(float)` - Frequency increase (default: 2.0, range: 1.0-4.0)
- `setColorRange(colorLow, colorHigh)` - Color mapping

## Gradient Texture
```cpp
// Linear Gradient
auto gradient = GradientTexture::create();
gradient->setSize(512, 512);
gradient->setType(GradientTexture::GradientType::Linear);
gradient->setAngle(0.0f);  // Radians
gradient->setColors(
    glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),  // Start
    glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)   // End
);
gradient->regenerate();

// Radial Gradient
gradient->setType(GradientTexture::GradientType::Radial);
gradient->setCenter(glm::vec2(0.5f, 0.5f));  // Normalized [0, 1]
gradient->regenerate();
```

**Parameters:**
- `setType(GradientType)` - Linear or Radial
- `setAngle(float)` - Gradient direction in radians (linear only)
- `setCenter(vec2)` - Center point (radial only)
- `setColors(start, end)` - Start and end colors

## Marble Texture
```cpp
auto marble = MarbleTexture::create();
marble->setSize(512, 512);
marble->setScale(1.0f);
marble->setOctaves(4);
marble->setColors(
    glm::vec4(0.9f, 0.9f, 0.9f, 1.0f),  // Light marble
    glm::vec4(0.3f, 0.3f, 0.3f, 1.0f)   // Dark veins
);
marble->regenerate();
```

**Parameters:**
- `setScale(float)` - Pattern scale (default: 1.0)
- `setOctaves(int)` - Detail levels (default: 4)
- `setColors(color1, color2)` - Light and dark colors

## Wood Texture
```cpp
auto wood = WoodTexture::create();
wood->setSize(512, 512);
wood->setScale(1.0f);
wood->setRings(10.0f);
wood->setColors(
    glm::vec4(0.6f, 0.4f, 0.2f, 1.0f),  // Light wood
    glm::vec4(0.3f, 0.2f, 0.1f, 1.0f)   // Dark grain
);
wood->regenerate();
```

**Parameters:**
- `setScale(float)` - Pattern scale (default: 1.0)
- `setRings(float)` - Number of growth rings (default: 10.0)
- `setColors(color1, color2)` - Light and dark wood colors

## Common Methods (All Procedural Textures)

```cpp
// Size
texture->setSize(width, height);
texture->setChannels(4);  // 3 for RGB, 4 for RGBA

// Generation
texture->regenerate();  // Generate and upload to GPU
texture->setNeedsRegeneration(true);

// Query
int w = texture->width();
int h = texture->height();

// All Texture methods also available
texture->setWrapS(GL_REPEAT);
texture->setWrapT(GL_REPEAT);
texture->setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
texture->setMagFilter(GL_LINEAR);
texture->setBlendMode(TextureBlendMode::Multiply);
texture->setBlendFactor(0.5f);
```

## Procedural Utility Functions

All functions are in the `ivf::ProcUtils` namespace:

```cpp
using namespace ivf::ProcUtils;

// Noise
float n = noise(x, y);
float f = fbm(x, y, octaves, persistence, lacunarity);
float t = turbulence(x, y, octaves);
float v = voronoi(x, y, cellSize);

// Patterns
float c = checkerboard(x, y, size);
float g = gradient(x, y, angle);
float r = radialGradient(x, y, centerX, centerY);
float m = marble(x, y, scale, octaves);
float w = wood(x, y, scale, rings);

// Utilities
float s = smoothstep(t);
float b = bilerp(a, b, c, d, tx, ty);
glm::vec4 color = mixColors(color1, color2, factor);
float clamped = clamp01(value);
float mapped = remap(value, inMin, inMax, outMin, outMax);
```

## Performance Tips

1. **Texture Size**: Start with 512x512, increase if needed
   - 256x256: Very fast, suitable for tiling patterns
   - 512x512: Good balance, recommended default
   - 1024x1024: Slower, use for high-detail textures
   - 2048x2048: Very slow, avoid unless necessary

2. **Octaves**: More octaves = more detail but slower
   - 1-2 octaves: Fast, simple patterns
   - 4 octaves: Good balance (recommended)
   - 6-8 octaves: Very detailed but slow

3. **Regeneration**: Only call `regenerate()` when needed
   - Change multiple parameters, then regenerate once
   - Use `setNeedsRegeneration()` for deferred updates

4. **Channels**: Use RGB (3) instead of RGBA (4) if alpha not needed
   - Saves 25% memory and bandwidth

## Integration Examples

### With Multitexturing
```cpp
auto base = PerlinNoiseTexture::create();
base->setBlendMode(TextureBlendMode::Normal);
base->regenerate();

auto detail = CheckerboardTexture::create();
detail->setBlendMode(TextureBlendMode::Multiply);
detail->setBlendFactor(0.3f);
detail->regenerate();

myBox->addTexture(base);
myBox->addTexture(detail);
myBox->setUseMultiTexturing(true);
```

### With Material Colors
```cpp
auto material = Material::create();
material->setDiffuseColor(glm::vec4(0.8f, 0.6f, 0.4f, 1.0f));
material->setUseTexture(true);

auto texture = WoodTexture::create();
texture->regenerate();

myBox->setMaterial(material);
myBox->addTexture(texture);
```

### Custom Procedural Texture
```cpp
class CustomTexture : public ProceduralTexture {
protected:
    void generate() override {
        for (int y = 0; y < m_height; y++) {
            for (int x = 0; x < m_width; x++) {
                float u = float(x) / m_width;
                float v = float(y) / m_height;
                setPixel(x, y, getPixel(u, v));
            }
        }
    }
    
    glm::vec4 getPixel(float u, float v) override {
        // Your custom procedural logic here
        return glm::vec4(u, v, 0.0f, 1.0f);
    }
};
```
