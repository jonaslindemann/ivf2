# Custom Effects

When none of the built-in effects gives the look you want, write your own. A custom effect is a small C++ class wrapped around a GLSL fragment shader. The shader runs once for every pixel on the screen, reads the image produced so far, and writes a new colour.

## Anatomy of an effect

Subclass `ivf::Effect` and override up to three methods:

| Method | Called | Purpose |
|--------|--------|---------|
| `doLoad()` | Once, by `load()` | Compile the shader program into `m_program` |
| `doUpdateParams()` | Every frame, before the chain runs | Send your parameters to the shader as uniforms |
| `setupProperties()` | When an inspector first asks | Expose parameters to the [effect inspector](index.md#effect-inspector) |

## Example: an invert-and-threshold effect

This effect turns the image into a high-contrast two-colour picture, with a parameter for the threshold and the two colours.

### The shader

Every effect uses the same full-screen vertex shader, `render_to_texture_vert_shader_source_330` from `<ivf/post_shaders.h>`. You only write the fragment shader:

```cpp
inline const std::string two_tone_frag = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;   // the image so far
uniform float time;                // scene time in seconds

uniform float threshold;
uniform vec3 darkColor;
uniform vec3 lightColor;

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    float lum = dot(col, vec3(0.2126, 0.7152, 0.0722));
    float t = smoothstep(threshold - 0.02, threshold + 0.02, lum);
    FragColor = vec4(mix(darkColor, lightColor, t), 1.0);
}
)";
```

### The class

```cpp
#include <ivf/effect.h>
#include <ivf/post_shaders.h>
#include <ivf/shader_manager.h>

class TwoToneEffect : public ivf::Effect {
private:
    float m_threshold{0.5f};
    glm::vec3 m_dark{0.05f, 0.05f, 0.15f};
    glm::vec3 m_light{1.0f, 0.85f, 0.4f};

public:
    TwoToneEffect() { setName("Two Tone"); }

    static std::shared_ptr<TwoToneEffect> create() { return std::make_shared<TwoToneEffect>(); }

    void setThreshold(float t) { m_threshold = t; }
    void setColors(glm::vec3 dark, glm::vec3 light) { m_dark = dark; m_light = light; }

protected:
    void doLoad() override
    {
        m_program = compileProgram(ivf::render_to_texture_vert_shader_source_330, two_tone_frag, "two_tone");
    }

    void doUpdateParams() override
    {
        m_program->uniformFloat("threshold", m_threshold);
        m_program->uniformVec3("darkColor", m_dark);
        m_program->uniformVec3("lightColor", m_light);
    }

    void setupProperties() override
    {
        addProperty("Threshold", &m_threshold, "Two Tone");
        addProperty("Dark colour", &m_dark, "Two Tone");
        addProperty("Light colour", &m_light, "Two Tone");
    }
};
```

`compileProgram()` registers the program with the shader manager without changing the program currently in use. If the shader fails to compile it returns `nullptr` and logs the compiler output; `addEffect()` then skips the effect.

### Using it

Exactly like a built-in effect:

```cpp
setRenderToTexture(true);

auto twoTone = TwoToneEffect::create();
twoTone->setThreshold(0.4f);
twoTone->load();
addEffect(twoTone);
```

Give each effect class a unique program name (`"two_tone"` above), because the shader manager identifies programs by name.

## What the shader can read

The post processor binds these inputs for every effect. Declare the ones you need; unused ones cost nothing.

| Uniform | Type | Contents |
|---------|------|----------|
| `screenTexture` | `sampler2D` | Output of the previous effect, or the rendered scene for the first effect |
| `time` | `float` | Scene time in seconds |
| `previousFrame` | `sampler2D` | The final image of the previous frame, for trails and feedback |
| `depthTexture` | `sampler2D` | Scene depth buffer, from 0 (near plane) to 1 (far plane) |
| `invProjection` | `mat4` | Inverse projection matrix, to turn depth into view-space positions |
| `invView` | `mat4` | Inverse view matrix, to go from view space to world space |

`TexCoords` runs from (0, 0) in the bottom-left corner to (1, 1) in the top-right. For sizes in pixels, use `textureSize(screenTexture, 0)`.

### Reconstructing positions from depth

Depth-aware effects such as fog need the 3D position behind each pixel:

```glsl
float depth = texture(depthTexture, TexCoords).r;
vec4 viewPos = invProjection * vec4(TexCoords * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
viewPos /= viewPos.w;
float distance = length(viewPos.xyz);          // distance from the camera
vec3 worldPos = (invView * viewPos).xyz;       // position in the scene
```

A depth of 1.0 means nothing was drawn at that pixel (the background).

## Tips

- **Start from a built-in effect.** All of their shaders are in `<ivf/post_shaders.h>`, and copying one is the quickest way to get the boilerplate right.
- **Keep loops small.** The shader runs for every pixel, every frame. A 20×20 sampling loop at 1080p is 800 million texture reads per frame.
- **Animate from C++.** Change parameters in `onUpdateEffects()` rather than hard-coding time-based behaviour in the shader, so the effect can be driven by tweens, automation or music.
- **Check the console.** Shader compile errors are printed when `load()` runs.

## Related

- [Post-processing overview](index.md)
- [Shader Materials](../core/shader_materials.md) — custom shaders on individual objects rather than the whole screen
