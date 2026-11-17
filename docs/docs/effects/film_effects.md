# Film and Distortion Effects

These effects imitate cameras, lenses and screens, or bend the image in ways that are hard to get any other way. All of them follow the pattern from the [overview](index.md): create, set parameters, `load()`, `addEffect()`. Effects with a speed or noise parameter animate on their own using the scene time.

## Film and lens

### FilmgrainEffect

Animated grain, as on photographic film. A small amount makes clean computer graphics feel more natural.

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setNoiseIntensity(n)` | 0.1 | Strength of the grain |
| `setGrainBlending(b)` | 0.1 | How strongly the grain is mixed into the image |

### VignetteEffect

Darkens the corners and edges, drawing the eye to the centre.

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setSize(s)` | 1.5 | Size of the bright centre; smaller gives a stronger vignette |
| `setSmoothness(s)` | 1.0 | Softness of the transition to the dark edges |

### ChromaticEffect

Chromatic aberration: the red, green and blue channels are shifted slightly apart, giving coloured fringes like a cheap lens.

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setOffset(o)` | 0.01 | Channel separation; the fringes grow towards the edges of the screen |

### ScanlineEffect

Horizontal lines like an old CRT monitor.

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setLineSpacing(px)` | 4.0 | Distance between lines in pixels |
| `setLineIntensity(i)` | 0.3 | How dark the lines are |
| `setScrollSpeed(s)` | 0.0 | Scrolls the lines vertically when above 0 |

### GlitchEffect

Blocks of the image jump sideways and split their colours, like a corrupted video signal.

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setIntensity(i)` | 0.05 | How far blocks are displaced |
| `setBlockSize(s)` | 0.1 | Height of the glitching bands, as a fraction of the screen |
| `setSpeed(s)` | 2.0 | How many times per second the glitch pattern changes |

## Distortion

Distortions move pixels around. Add them after [fog](blur.md#fog) and [depth of field](blur.md#depth-of-field), because those effects rely on each pixel still being where the depth buffer says it is.

### PixelationEffect

Replaces the image with large square pixels.

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setPixelSize(px)` | 4.0 | Size of each block in screen pixels |

### WaveDistortionEffect

Ripples the image as if seen through water or heat haze.

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setFrequency(f)` | 10.0 | Spatial frequency; higher gives more, tighter ripples |
| `setAmplitude(a)` | 0.01 | Size of the displacement, as a fraction of the screen |
| `setSpeed(s)` | 1.0 | How fast the waves move |

### SwirlEffect

Twists the image around the centre of the screen.

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setRadius(r)` | 0.5 | Radius of the swirl, as a fraction of the screen |
| `setAngle(a)` | 3.14159 | Amount of twist at the centre, in radians |

### KaleidoscopeEffect

Mirrors a wedge of the image around the centre, like a kaleidoscope. Combined with slow scene motion, it turns almost anything into a symmetrical pattern.

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setSegments(n)` | 6.0 | Number of mirrored segments |
| `setRotation(r)` | 0.0 | Rotation of the pattern; animate it for a turning kaleidoscope |

### FeedbackEffect

Video feedback: each frame is mixed with a slightly zoomed, rotated and faded copy of the previous one. The results range from soft trails to infinite tunnels and spirals, a classic creative-coding look.

```cpp
auto feedback = ivf::FeedbackEffect::create();
feedback->setFeedbackAmount(0.9f);
feedback->setZoom(1.01f);          // zoom in a little each frame
feedback->setRotation(0.005f);     // radians per frame
feedback->setColorDecay({0.98f, 0.95f, 0.9f});
feedback->load();
addEffect(feedback);
```

| Parameter | Description |
|-----------|-------------|
| `setFeedbackAmount(a)` | How much of the previous frame is kept; close to 1 gives long trails |
| `setZoom(z)` | Scale applied to the previous frame each time; above 1 zooms inwards |
| `setRotation(rad)` | Rotation applied to the previous frame each time |
| `setOffset(x, y)` | Drift applied to the previous frame each time |
| `setColorDecay(rgb)` | Per-channel fading of the fed-back image; uneven values give colour trails |

Feedback reads the previous frame's final image, so its look depends on everything else in the chain. Clear backgrounds and bright, moving objects work best.

## Related

- [Post-processing overview](index.md)
- [Colour Effects](color_effects.md)
- [Effects Examples](../examples/effects_examples.md)
