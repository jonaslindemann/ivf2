# Colour Effects

These effects change the colours and tones of the image: from subtle grading to strongly stylised looks such as posterised, printed or night-vision images. All of them follow the pattern from the [overview](index.md): create, set parameters, `load()`, `addEffect()`.

## TintEffect

Converts the image towards greyscale and then colours it, like a photo filter. With a warm tint colour it gives a sepia look.

```cpp
auto tint = ivf::TintEffect::create();
tint->setTintColor({1.2f, 0.9f, 0.7f});   // warm sepia
tint->setTintStrength(0.5f);
tint->load();
addEffect(tint);
```

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setTintColor(rgb)` | (1.2, 0.9, 0.7) | Colour multiplied into the greyscale image |
| `setTintStrength(s)` | 0.5 | Mix between the original (0) and fully tinted (1) image |
| `setGrayScaleWeights(rgb)` | (0.299, 0.587, 0.114) | How red, green and blue contribute to brightness |

## ColorGradingEffect

Adjusts colour the way a film colourist does: separate colour shifts for the dark, middle and bright parts of the image, plus overall contrast and saturation.

```cpp
auto grade = ivf::ColorGradingEffect::create();
grade->setShadows({0.45f, 0.5f, 0.6f});     // cool shadows
grade->setHighlights({0.6f, 0.5f, 0.45f});  // warm highlights
grade->setContrast(1.1f);
grade->setSaturation(0.9f);
grade->load();
addEffect(grade);
```

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setShadows(rgb)` | (0.6, 0.5, 0.5) | Colour shift in dark areas; 0.5 per channel is neutral, above adds that colour |
| `setMidtones(rgb)` | (0.5, 0.5, 0.5) | Colour shift in middle tones |
| `setHighlights(rgb)` | (0.5, 0.5, 0.6) | Colour shift in bright areas |
| `setContrast(c)` | 1.0 | 1 is unchanged; higher increases contrast |
| `setSaturation(s)` | 1.0 | 0 is greyscale, 1 is unchanged, above 1 is more vivid |

The defaults are not neutral: they give slightly warm shadows and cool highlights. Set all three zones to (0.5, 0.5, 0.5) to start from an unchanged image.

## PosterizeEffect

Reduces each colour channel to a few levels, giving flat bands of colour like a screen print.

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setLevels(n)` | 4.0 | Number of levels per channel |

## DitheringEffect

Adds a fine dither pattern. Subtly, this hides colour banding in smooth gradients such as skies and fog. Combined with `PosterizeEffect`, it gives a retro, low-colour look. It has no parameters.

## HalftoneEffect

Redraws the image as a grid of dots whose size follows the brightness, like newspaper or comic printing.

| Parameter | Description |
|-----------|-------------|
| `setDotSize(px)` | Size of each dot cell in pixels |
| `setAngle(rad)` | Rotation of the dot grid |
| `setGrayscale(on)` | One monochrome screen (true), or a rotated screen per colour channel like CMYK printing (false) |

## NightVisionEffect

Recolours the image in green phosphor with noise and glow, like a night-vision scope.

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setNoiseIntensity(n)` | 0.05 | Amount of animated noise |
| `setGlowStrength(g)` | 0.3 | Glow around bright areas |
| `setPhosphorColor(rgb)` | (0.1, 1.0, 0.1) | Colour of the display |

## EdgeDetectionEffect

Finds edges in the image and draws them as lines, for a technical-drawing or outline look. It has no parameters.

## FadeEffect

Blends the image towards a solid colour. Animate the amount from 0 to 1 for fade-outs between scenes.

| Parameter | Default | Description |
|-----------|---------|-------------|
| `setFadeColor(rgb)` | (0, 0, 0) | Colour to fade to |
| `setFadeAmount(a)` | 0.0 | 0 shows the scene, 1 shows only the fade colour |

```cpp
void onUpdateEffects() override
{
    m_fade->setFadeAmount(m_fadeTween->value());   // driven by a Tween, for example
}
```

## Related

- [Post-processing overview](index.md)
- [Film and Distortion Effects](film_effects.md)
- [Effects Examples](../examples/effects_examples.md)
