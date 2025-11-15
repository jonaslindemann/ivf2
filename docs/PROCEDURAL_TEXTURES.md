# Procedural Textures Implementation for IVF2

## Overview

This implementation adds CPU-based procedural texture generation capabilities to the IVF2 library. The system allows for creating textures programmatically using mathematical functions and noise algorithms, without requiring external image files.

## Architecture

### Core Components

1. **proc_utils.h/cpp** - Procedural utility functions
   - Perlin noise implementation
   - Fractional Brownian Motion (fBm)
   - Voronoi/Worley noise
   - Turbulence and other pattern generators
   - Color mixing and interpolation utilities

2. **procedural_texture.h/cpp** - Procedural texture classes
   - `ProceduralTexture` - Abstract base class
   - `CheckerboardTexture` - Checkerboard pattern
   - `PerlinNoiseTexture` - Multi-octave Perlin noise
   - `GradientTexture` - Linear and radial gradients
   - `MarbleTexture` - Marble pattern using noise
   - `WoodTexture` - Wood grain pattern

3. **procedural1 example** - Interactive demonstration
   - Real-time parameter adjustment
   - Multiple texture type selection
   - Live regeneration and preview

## Design Principles

### 1. Inheritance from Texture Class
All procedural textures extend the existing `Texture` class, ensuring compatibility with the current texture system. They can be used anywhere a regular `Texture` is expected.

### 2. Lazy Generation Pattern
Textures are only regenerated when parameters change or when explicitly requested:

```cpp
auto noise = PerlinNoiseTexture::create();
noise->setSize(512, 512);
noise->setScale(4.0f);
noise->setOctaves(6);
noise->regenerate(); // Generate and upload to GPU
```

### 3. Parameter Dirty Tracking
The `m_needsRegeneration` flag ensures textures are only regenerated when necessary, avoiding expensive CPU operations.

### 4. Separation of Concerns
- **Generation** (`generate()`) - Creates pixel data in CPU memory
- **Upload** (`upload()`) - Transfers data to GPU
- **Parameter setters** - Mark texture as needing regeneration

## Usage Examples

### Basic Checkerboard

```cpp
auto checker = CheckerboardTexture::create();
checker->setSize(512, 512);
checker->setCheckerSize(32);
checker->setColors(
    glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),  // Red
    glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)   // Green
);
checker->regenerate();

// Use like any texture
myBox->addTexture(checker);
```

### Perlin Noise Terrain

```cpp
auto noise = PerlinNoiseTexture::create();
noise->setSize(1024, 1024);
noise->setScale(8.0f);
noise->setOctaves(6);
noise->setPersistence(0.5f);
noise->setColorRange(
    glm::vec4(0.2f, 0.4f, 0.1f, 1.0f),  // Dark green
    glm::vec4(0.9f, 0.9f, 0.8f, 1.0f)   // Light tan
);
noise->regenerate();
```

### Radial Gradient

```cpp
auto gradient = GradientTexture::create();
gradient->setSize(512, 512);
gradient->setType(GradientTexture::GradientType::Radial);
gradient->setCenter(glm::vec2(0.5f, 0.5f));
gradient->setColors(
    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),  // White center
    glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)   // Transparent edges
);
gradient->regenerate();
```

### Marble Pattern

```cpp
auto marble = MarbleTexture::create();
marble->setSize(512, 512);
marble->setScale(2.0f);
marble->setOctaves(4);
marble->setColors(
    glm::vec4(0.9f, 0.9f, 0.9f, 1.0f),
    glm::vec4(0.3f, 0.3f, 0.3f, 1.0f)
);
marble->regenerate();
```

## Procedural Utilities API

### Noise Functions

- `noise(x, y)` - Basic 2D Perlin noise
- `fbm(x, y, octaves, persistence, lacunarity)` - Fractional Brownian Motion
- `voronoi(x, y, cellSize)` - Voronoi/cellular noise
- `turbulence(x, y, octaves)` - Absolute value noise

### Pattern Functions

- `checkerboard(x, y, size)` - Checkerboard pattern
- `gradient(x, y, angle)` - Linear gradient
- `radialGradient(x, y, cx, cy)` - Radial gradient
- `marble(x, y, scale, octaves)` - Marble pattern
- `wood(x, y, scale, rings)` - Wood grain pattern

### Utility Functions

- `smoothstep(t)` - Smooth interpolation
- `bilerp(a, b, c, d, tx, ty)` - Bilinear interpolation
- `mixColors(color1, color2, factor)` - Color blending
- `clamp01(value)` - Clamp to [0, 1]
- `remap(value, inMin, inMax, outMin, outMax)` - Range mapping

## Performance Considerations

### CPU-Side Generation
- Textures are generated on the CPU, which can be slow for large textures
- Typical generation times (on modern CPU):
  - 512x512 checkerboard: ~1-2ms
  - 512x512 Perlin noise (4 octaves): ~10-20ms
  - 1024x1024 Perlin noise (6 octaves): ~80-150ms

### Optimization Strategies

1. **Size Management**: Use appropriate texture sizes for your needs
2. **Lazy Regeneration**: Only regenerate when parameters change
3. **Caching**: Keep generated data in memory if parameters don't change
4. **Thread Safety**: Generation can be moved to background threads in future

### When to Use Procedural Textures

**Good Use Cases:**
- Runtime texture customization
- Memory-efficient repeating patterns
- Algorithmic art and effects
- Prototyping and testing
- Texture synthesis for missing assets

**When to Use Image Files Instead:**
- Photo-realistic textures
- Artist-created content
- Textures that don't need runtime modification
- Very large, complex textures

## Integration with Existing IVF2 Features

### Multitexturing Support
Procedural textures work seamlessly with multitexturing:

```cpp
auto diffuse = PerlinNoiseTexture::create();
diffuse->setBlendMode(TextureBlendMode::Normal);
diffuse->regenerate();

auto detail = CheckerboardTexture::create();
detail->setBlendMode(TextureBlendMode::Multiply);
detail->setBlendFactor(0.5f);
detail->regenerate();

myBox->addTexture(diffuse);
myBox->addTexture(detail);
myBox->setUseMultiTexturing(true);
```

### Material System
Procedural textures respect material colors and lighting:

```cpp
auto material = Material::create();
material->setDiffuseColor(glm::vec4(0.8f, 0.6f, 0.4f, 1.0f));
material->setUseTexture(true);

myBox->setMaterial(material);
myBox->addTexture(proceduralTexture);
```

## Future Enhancements

### Potential GPU-Based Implementation
For real-time performance, consider adding shader-based procedural textures:

```cpp
class ShaderProceduralTexture : public Texture {
    // Generate texture using compute shaders or FBO rendering
    void generateOnGPU();
};
```

### Additional Pattern Types
- Cellular/Voronoi patterns with distance metrics
- Fractal patterns (Mandelbrot, Julia sets)
- Brick/tile patterns
- Organic patterns (clouds, terrain)

### Animation Support
Add time parameter for animated procedural textures:

```cpp
class AnimatedProceduralTexture : public ProceduralTexture {
    void update(float time);
};
```

### Property Inspectable Support
Integrate with the property system for editor support:

```cpp
class ProceduralTexture : public Texture, public PropertyInspectable {
    void registerProperties() override;
};
```

## Example Application

The `procedural1` example demonstrates all procedural texture types with interactive controls:

- Switch between texture types in real-time
- Adjust parameters with immediate visual feedback
- Modify texture resolution
- Customize colors for each pattern type

Run it with:
```bash
./procedural1
```

## Files Added

### Headers
- `include/ivf/proc_utils.h` - Procedural utility functions
- `include/ivf/procedural_texture.h` - Procedural texture classes

### Implementation
- `src/ivf/proc_utils.cpp` - Utility implementations
- `src/ivf/procedural_texture.cpp` - Texture class implementations

### Examples
- `examples/procedural1/procedural1.cpp` - Interactive demo
- `examples/procedural1/CMakeLists.txt` - Build configuration

## Conclusion

This implementation provides a solid foundation for procedural texture generation in IVF2. The system is:

- **Compatible** - Works with existing texture infrastructure
- **Extensible** - Easy to add new procedural types
- **Performant** - Optimized with lazy regeneration
- **Flexible** - Supports various pattern types and parameters
- **Well-documented** - Comprehensive API documentation and examples

The CPU-based approach is appropriate for moderate-sized textures that need runtime customization. For high-performance applications requiring large textures or real-time updates, consider implementing GPU-based generation in the future.
