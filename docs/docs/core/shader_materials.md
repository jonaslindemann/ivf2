# Shader Materials

A regular `Material` describes how a surface reacts to light: its colours and shininess. A shader material instead lets you write the GLSL program that draws the surface yourself. Use one when you want a look that lighting alone can't give, such as glowing objects, animated surfaces, holograms, or a procedural sky.

ivf2 has three levels, from most to least work:

| Class | Use it for |
|-------|------------|
| `ShaderMaterial` | Any custom vertex and fragment shader on a node |
| `BlendShaderMaterial` | The same, drawn with additive or alpha blending: glows, light trails, veils |
| `GlowMaterial`, `MistMaterial`, `GradientSkyMaterial`, `WaterMaterial` | Ready-made shaders with typed setters, no GLSL needed |

## Ready-made materials

### GlowMaterial

Makes a shape look like a ball of light: brightest where it faces you, fading towards the silhouette. Lights are ignored. Overlapping glows add up, and with [`MipBloomEffect`](../effects/blur.md#mipbloomeffect) they get a soft halo.

```cpp
auto glow = ivf::GlowMaterial::create({1.0f, 0.6f, 0.2f}, 1.5f);   // colour, brightness
auto orb = ivf::Sphere::create(0.3);
orb->setMaterial(glow);
```

| Setter | Description |
|--------|-------------|
| `setColor(rgb)` | Glow colour |
| `setBrightness(b)` | Multiplier for the colour; above 1 drives bloom |
| `setFalloff(f)` | How quickly the glow fades towards the edge (default 1.5) |

All glow materials share one shader program, so a separate material per colour is cheap.

### MistMaterial

A soft, translucent layer of mist for flat shapes such as a `Plane` or `Disk`. It is densest at the node's origin and fades to fully transparent at the radius, so the edges of the shape never show. Move, rotate and scale the node to place it.

```cpp
auto mist = ivf::MistMaterial::create({0.5f, 0.45f, 0.7f}, 0.35f, 12.0f);   // colour, opacity, radius
auto layer = ivf::Plane::create(24.0, 24.0);
layer->setMaterial(mist);
```

| Setter | Description |
|--------|-------------|
| `setColor(rgb)` | Mist colour |
| `setOpacity(a)` | Opacity at the centre, 0–1 |
| `setRadius(r)` | Distance from the node's origin where the mist has faded out, in local units |
| `setSoftness(s)` | How gradually it fades (default 0.8) |

For fog over the whole scene, use [`FogEffect`](../effects/blur.md#fog) instead.

### GradientSkyMaterial

A procedural sky for a dome: a gradient from horizon to zenith, an optional glowing band along the horizon, and optional twinkling stars.

```cpp
auto sky = ivf::GradientSkyMaterial::create({0.0f, 0.0f, 0.02f}, {0.1f, 0.08f, 0.2f});   // zenith, horizon
sky->setStarIntensity(1.0f);

auto dome = ivf::Sphere::create(50.0, 64, 32);
dome->setMaterial(sky);
add(dome);   // add the sky first
```

| Setter | Description |
|--------|-------------|
| `setZenithColor(rgb)` | Colour straight up |
| `setHorizonColor(rgb)` | Colour at the horizon |
| `setGroundColor(rgb)` | Colour below the horizon |
| `setGradientExponent(e)` | Shape of the gradient (default 0.5) |
| `setGlowColor(rgb)` | Colour of a band along the horizon; black for none (default) |
| `setGlowSharpness(s)` | Narrowness of the horizon band (default 10) |
| `setStarIntensity(i)` | Brightness of the stars; 0 hides them (default) |

The dome always stays centred on the camera, so it never comes closer; its radius only has to fit inside the camera's far plane. It is drawn without writing depth, so everything added later covers it and depth-based effects such as fog still see empty sky. Stars twinkle with the global `time` uniform (see [below](#global-uniforms)).

### WaterMaterial

A reflective water surface with animated ripples. It needs a `PlanarReflection`, which renders the scene mirrored in the water plane:

```cpp
auto lake = ivf::Plane::create(80.0, 80.0, 4, 4);
add(lake);

m_reflection = ivf::PlanarReflection::create({0, 0, 0}, {0, 1, 0});   // point and normal of the plane
m_reflection->addHiddenNode(lake);   // the water shouldn't reflect itself
addReflection(m_reflection);

m_water = ivf::WaterMaterial::create(m_reflection);
lake->setMaterial(m_water);
```

| Setter | Description |
|--------|-------------|
| `setWaterColor(rgb)` | Colour seen looking straight down |
| `setReflectivity(r)` | Reflection at grazing angles, 0–1 |
| `setMinReflectivity(r)` | Reflection looking straight down, 0–1 |
| `setFresnelPower(p)` | How quickly reflection increases towards grazing angles |
| `setRippleStrength(s)` | Height of the ripples; 0 gives a perfect mirror |
| `setRippleScale(s)` | Ripples per world unit |
| `setRippleSpeed(s)` | Animation speed of the ripples |
| `setDistortion(d)` | How much the ripples bend the reflection |

Ripples animate with the `time` uniform. See the `reflection1` example for a full scene.

## Writing your own: ShaderMaterial

### Creating the material

Load the shaders from files or from strings in your code:

```cpp
// From files, reloaded automatically when you save them
m_glow = ivf::ShaderMaterial::createFromFiles("shaders/glow.vert", "shaders/glow.frag", "glow", true);

// From strings
m_mat = ivf::ShaderMaterial::createFromStrings(vertSource, fragSource, "my_shader");

sphere->setMaterial(m_glow);
```

With `watch` set to `true` (the last argument of `createFromFiles`), the window reloads the shader whenever the file changes. Edit the GLSL, save, and see the result immediately without restarting.

Shader files in `shaders/` are copied next to the executable at build time. When you add a new one, register it in the asset copy rules in `CMakeLists.txt`.

### What the shader receives

Vertex attributes come at these fixed locations:

```glsl
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTex;
layout (location = 3) in vec3 aNormal;
```

`uniform mat4 model` (object to world) is set automatically. The camera comes from the `IvfCamera` uniform block, which the render context keeps up to date for all programs; declare it in every stage that uses one of its members:

```glsl
layout(std140) uniform IvfCamera {
    mat4 view;           // world to view space
    mat4 projection;     // view to clip space
    mat4 viewProjection; // projection * view
    vec3 viewPos;        // camera position in world space
};
```

The block has no instance name, so the members are used like plain uniforms (`projection * view * p`). In C++ shader strings, `IVF_CAMERA_BLOCK_GLSL` from `<ivf/uniform_blocks.h>` expands to this declaration. Older shaders that declare plain `uniform mat4 view, projection;` and `uniform vec3 viewPos;` instead still get them set (the `ripple` shader does); a program can't have both.

The standard light and material uniforms are only uploaded after `setUseLighting(true)`, so unlit shaders don't need to declare them. A lit shader gets the lights from the `IvfLights` uniform block: in C++ shader strings, `IVF_LIGHTS_BLOCK_GLSL` declares the `PointLight`, `DirLight`, `SpotLight` and `ShadowData` structs and the block with the arrays `pointLights`, `dirLights`, `spotLights`, `shadows` and the counts `pointLightCount`, `dirLightCount`, `spotLightCount` (see `include/ivf/uniform_blocks.h` for the members). For shadows, append `ivf::shadow_glsl` (`ivf/shadow_shaders.h`) and call `ivfShadowFactor(light.shadowIndex, fragPos, normal, toLight)`; see [Shadows](../advanced/shadows.md#in-custom-shaders).Shaders that declare the light arrays as plain uniforms (the layout before the block) still get every light uploaded.

### A complete example

The `glow` shader from the `shader_material1` example: the sphere breathes along its normals and glows at the rim.

```glsl
// glow.vert
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 3) in vec3 aNormal;

uniform mat4 model;
layout(std140) uniform IvfCamera {
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec3 viewPos;
};
uniform float time;
uniform float pulse;

out vec3 worldPos;
out vec3 worldNormal;

void main()
{
    float s = 1.0 + pulse * 0.08 * sin(time * 2.0 + aPos.y * 4.0);
    vec4 p = model * vec4(aPos * s, 1.0);
    worldPos = p.xyz;
    worldNormal = normalize(mat3(model) * aNormal);
    gl_Position = projection * view * p;
}
```

```glsl
// glow.frag
#version 330 core
in vec3 worldPos;
in vec3 worldNormal;

layout(std140) uniform IvfCamera {
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec3 viewPos;
};
uniform vec3 glowColor;
uniform float time;
uniform float pulse;

out vec4 fragColor;

void main()
{
    vec3 v = normalize(viewPos - worldPos);
    float rim = pow(1.0 - max(dot(normalize(worldNormal), v), 0.0), 2.5);
    float core = 0.25 + 0.15 * pulse * sin(time * 2.0);
    fragColor = vec4(glowColor * (core + 1.5 * rim), 1.0);
}
```

### Uniforms and textures

`setUniform()` sets a value for one material. It accepts `float`, `int`, `bool`, `glm::vec2`, `vec3`, `vec4`, `mat3` and `mat4`:

```cpp
m_glow->setUniform("glowColor", glm::vec3(0.4f, 1.0f, 0.8f));
m_glow->setUniform("pulse", 0.5f);
```

Bind textures to samplers with a texture unit:

```cpp
m_mat->setTexture("noiseTex", noiseTexture, 1);
```

### Global uniforms

`ShaderMaterial::setGlobalUniform()` sets a value for *every* shader material at once. It is the easy way to share time, music envelopes or other scene-wide values:

```cpp
void onUpdate() override
{
    float t = static_cast<float>(ivf::TimeController::instance()->elapsed());
    ivf::ShaderMaterial::setGlobalUniform("time", t);
}
```

A shader that doesn't declare a uniform simply ignores it, so global uniforms are safe to set even if only some shaders use them.

### Children and shadows

Child nodes of a node with a shader material are drawn with the same program, unless they have a shader material of their own. Shadow maps are drawn with the stock depth program for every node, so a vertex shader that moves vertices does not move the node's shadow; `setCastShadows(false)` leaves a node out of the shadow maps.

## Blending: BlendShaderMaterial

`BlendShaderMaterial` is a `ShaderMaterial` that blends its output with what is already on screen instead of covering it, and doesn't write depth. Create it the same way, with an extra blend mode:

```cpp
auto motes = ivf::BlendShaderMaterial::createFromFiles(
    "shaders/mote.vert", "shaders/mote.frag", "mote", ivf::BlendMode::Additive, true);
particles->setMaterial(motes);
```

| Mode | Result | Typical use |
|------|--------|-------------|
| `BlendMode::Additive` | Colours add up, so overlaps get brighter | Glowing particles, light trails, sparks |
| `BlendMode::Alpha` | Uses the shader's alpha for transparency | Veils, mist, glass |
| `BlendMode::Background` | Opaque, but writes no depth | Sky domes drawn first |

!!! note "Draw order is automatic"
    Blended nodes are in the transparent render layer: the scene window draws them after all opaque nodes, sorted back to front, whatever order they were added in. A `Background` sky is in the background layer and drawn first. A node can be moved to another layer with `setRenderLayer()`.

## Related

- [Materials and Textures](materials_and_textures.md) — standard lit materials
- [Custom Effects](../effects/custom_effects.md) — shaders for the whole screen instead of single objects
- `shader_material1`, `reflection1` and `mip_bloom1` examples
