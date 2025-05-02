# ivf2 Documentation Structure

## Directory Structure

```
docs/
├── index.md                      # Home page/introduction to ivf2
├── getting_started/              # Getting started guide
│   ├── installation.md           # Installation instructions
│   ├── basic_concepts.md         # Core concepts and architecture
│   ├── hello_world.md            # First example explained (hello_ivf)
│   └── building_examples.md      # How to build and run the examples
├── core/                         # Core functionality
│   ├── nodes.md                  # Node system and scene graph
│   ├── transforms.md             # Transformation system
│   ├── materials.md              # Material system
│   ├── lighting.md               # Lighting system
│   ├── textures.md               # Texture handling
│   └── shaders.md                # Shader system
├── primitives/                   # 3D primitives
│   ├── basic_primitives.md       # Cubes, spheres, etc.
│   ├── complex_primitives.md     # More complex shapes
│   └── text.md                   # Text rendering
├── effects/                      # Post-processing effects
│   ├── blur.md                   # Blur effects
│   ├── color_effects.md          # Tint, chromatic, etc.
│   ├── film_effects.md           # Film grain, vignette, etc.
│   └── custom_effects.md         # Creating custom effects
├── animation/                    # Animation system
│   ├── basic_animation.md        # Simple animations
│   ├── spline_animation.md       # Spline-based animation
│   ├── keyframe_animation.md     # Keyframe animation
│   └── lissajous.md              # Lissajous curves and patterns
├── ui/                           # UI system (imgui-based)
│   ├── ui_windows.md             # Creating UI windows
│   ├── widgets.md                # UI widgets and controls
│   ├── scene_ui_integration.md   # Integrating UI with scenes
│   └── custom_ui.md              # Creating custom UI elements
├── advanced/                     # Advanced topics
│   ├── shadows.md                # Shadow mapping
│   ├── model_loading.md          # Loading external models (assimp)
│   ├── selection.md              # Object selection system
│   └── visitor_pattern.md        # Using the visitor pattern
├── api/                          # API reference
│   ├── ivf.md                    # ivf namespace reference
│   ├── ivfui.md                  # ivfui namespace reference
│   └── ivfmath.md                # ivfmath namespace reference
└── examples/                     # Example walkthrough
    ├── animation_examples.md     # Animation examples explained
    ├── lighting_examples.md      # Lighting examples explained
    ├── effects_examples.md       # Effects examples explained
    ├── ui_examples.md            # UI examples explained
    └── advanced_examples.md      # More complex examples explained
```

## MkDocs Configuration File (mkdocs.yml)

```yaml
site_name: ivf2 Documentation
site_description: Documentation for the ivf2 C++ graphics library
site_author: Your Name/Organization

theme:
  name: material
  palette:
    primary: indigo
    accent: indigo
  features:
    - navigation.tabs
    - navigation.expand
    - navigation.instant
    - navigation.tracking
    - content.code.copy
    - content.code.annotate

markdown_extensions:
  - pymdownx.highlight
  - pymdownx.superfences
  - pymdownx.inlinehilite
  - pymdownx.tabbed
  - pymdownx.arithmatex
  - admonition
  - toc:
      permalink: true

nav:
  - Home: index.md
  - Getting Started:
    - Installation: getting_started/installation.md
    - Basic Concepts: getting_started/basic_concepts.md
    - Hello World: getting_started/hello_world.md
    - Building Examples: getting_started/building_examples.md
  - Core:
    - Nodes: core/nodes.md
    - Transforms: core/transforms.md
    - Materials: core/materials.md
    - Lighting: core/lighting.md
    - Textures: core/textures.md
    - Shaders: core/shaders.md
  - Primitives:
    - Basic Primitives: primitives/basic_primitives.md
    - Complex Primitives: primitives/complex_primitives.md
    - Text: primitives/text.md
  - Effects:
    - Blur: effects/blur.md
    - Color Effects: effects/color_effects.md
    - Film Effects: effects/film_effects.md
    - Custom Effects: effects/custom_effects.md
  - Animation:
    - Basic Animation: animation/basic_animation.md
    - Spline Animation: animation/spline_animation.md
    - Keyframe Animation: animation/keyframe_animation.md
    - Lissajous: animation/lissajous.md
  - UI:
    - UI Windows: ui/ui_windows.md
    - Widgets: ui/widgets.md
    - Scene UI Integration: ui/scene_ui_integration.md
    - Custom UI: ui/custom_ui.md
  - Advanced:
    - Shadows: advanced/shadows.md
    - Model Loading: advanced/model_loading.md
    - Selection: advanced/selection.md
    - Visitor Pattern: advanced/visitor_pattern.md
  - API Reference:
    - ivf Namespace: api/ivf.md
    - ivfui Namespace: api/ivfui.md
    - ivfmath Namespace: api/ivfmath.md
  - Examples:
    - Animation Examples: examples/animation_examples.md
    - Lighting Examples: examples/lighting_examples.md
    - Effects Examples: examples/effects_examples.md
    - UI Examples: examples/ui_examples.md
    - Advanced Examples: examples/advanced_examples.md

plugins:
  - search
  - minify:
      minify_html: true
```

## Page Templates

### index.md

```markdown
# ivf2 Library

Welcome to the ivf2 documentation. ivf2 is a modern C++ graphics and visualization library built on top of OpenGL.

## Features

- Modern, object-oriented C++ design
- Powerful scene graph management
- Extensive set of 3D primitives
- Flexible material system
- Dynamic lighting system
- Animation capabilities
- Post-processing effects
- ImGui-based user interface
- Model loading via Assimp
- Shadow mapping
- And more!

## Getting Started

Visit the [Getting Started](getting_started/installation.md) section to begin using ivf2.

## Examples

Check out the [Examples](examples/animation_examples.md) section to see ivf2 in action.
```

### getting_started/installation.md

```markdown
# Installation

This guide will help you install and set up the ivf2 library for your project.

## Prerequisites

- CMake 3.12+
- C++17 compatible compiler
- OpenGL 3.3+
- GLFW3
- GLEW
- ImGui
- Assimp (for model loading)
- Freetype (for text rendering)

## Dependencies

The following dependencies are required to build ivf2:

- GLFW (window management)
- GLEW (OpenGL extension loading)
- GLM (mathematics)
- ImGui (user interface)
- Assimp (model loading)
- Freetype (font rendering)
- stb_image (image loading)

## Building from Source

1. Clone the repository:
   ```
   git clone https://github.com/your-username/ivf2.git
   cd ivf2
   ```

2. Create a build directory:
   ```
   mkdir build
   cd build
   ```

3. Run CMake:
   ```
   cmake ..
   ```

4. Build the library:
   ```
   cmake --build .
   ```

5. Install the library (optional):
   ```
   cmake --install .
   ```

## Integrating with Your Project

### Using CMake

If you're using CMake, you can integrate ivf2 with your project like this:

```cmake
find_package(ivf2 REQUIRED)
target_link_libraries(your_project PRIVATE ivf2::ivf2)
```

### Manual Integration

If you're not using CMake, make sure to:

1. Include the ivf2 headers in your project
2. Link against the ivf2 library
3. Set up the appropriate include paths
```

### core/nodes.md

```markdown
# Node System

The ivf2 node system is the foundation of the scene graph hierarchy in the library.

## Overview

The node system in ivf2 follows a classic scene graph architecture, where each node can have child nodes and a single parent node. This hierarchical structure allows for complex scenes to be built from simple components.

## Node Base Class

The `Node` class is the base class for all scene graph elements in ivf2. It provides common functionality such as:

- Position, rotation, and scale transformations
- Parent-child relationships
- Rendering capabilities
- Material assignment
- Visibility control

## Key Node Types

### TransformNode

The `TransformNode` class allows for coordinate transformations that affect all of its children.

```cpp
auto transform = ivf::Transform::create();
transform->setPos(glm::vec3(1.0, 0.0, 0.0));
transform->setRotAxis(glm::vec3(0.0, 1.0, 0.0));
transform->setRotAngle(45.0);
```

### CompositeNode

The `CompositeNode` class is a container for multiple child nodes, implementing the Composite pattern.

```cpp
auto composite = ivf::CompositeNode::create();
composite->add(node1);
composite->add(node2);
```

### Other Node Types

- `Geometry` - Base class for all geometric entities
- `Light` - Base class for light sources
- `Camera` - Defines the viewpoint and projection

## Working with Nodes

### Creating Nodes

Most node classes in ivf2 follow the factory pattern with static `create()` methods:

```cpp
auto cube = ivf::Cube::create();
auto sphere = ivf::Sphere::create(1.0f); // radius
```

### Node Hierarchy

Building a scene graph hierarchy:

```cpp
auto scene = ivf::CompositeNode::create();
auto transform = ivf::Transform::create();
auto cube = ivf::Cube::create();

transform->add(cube);
scene->add(transform);
```
```

### animation/basic_animation.md

```markdown
# Basic Animation

This guide introduces the animation system in ivf2.

## Overview

ivf2 provides several ways to animate objects in your scene. The simplest form is direct manipulation of transform properties (position, rotation, scale) over time.

## Animation Basics

### Using the Update Loop

The simplest way to animate objects is to modify properties in the `onUpdate()` method:

```cpp
class MyWindow : public ivfui::GLFWSceneWindow {
private:
    ivf::CubePtr m_cube;
    
public:
    // ... constructor and other methods ...
    
    virtual void onUpdate() override {
        m_cube->setRotAxis(glm::vec3(1.0, 1.0, 1.0));
        m_cube->setRotAngle(20.0 * elapsedTime());
    }
};
```

### Time-Based Animation

Always use time-based animation instead of frame-based animation to ensure consistent speed across different systems:

```cpp
// Bad - frame-based animation
m_angle += 1.0;
object->setRotAngle(m_angle);

// Good - time-based animation
object->setRotAngle(20.0 * elapsedTime());
```

## Animation Types

ivf2 offers several built-in animation types:

1. **Direct manipulation** - Updating properties each frame
2. **SplineAnimation** - Animation along a spline path
3. **KeyframeAnimation** - Animation between key poses
4. **Procedural animation** - Using mathematical functions
```

### examples/animation_examples.md

```markdown
# Animation Examples

This section explores the animation examples provided with ivf2.

## animation1 Example

The `animation1` example demonstrates simple animations using direct manipulation within the update loop.

### Key Concepts Demonstrated

- Basic rotation animation
- Time-based animation
- Multiple animated objects

### Code Walkthrough

```cpp
virtual void onUpdate()
{
    m_cube->setRotAxis(glm::vec3(1.0, 1.0, 1.0));
    m_cube->setRotAngle(20.0 * elapsedTime());

    m_sphereXfm->setRotAxis(glm::vec3(0.0, 1.0, 0.0));
    m_sphereXfm->setRotAngle(20.0 * elapsedTime());
}
```

The above code rotates a cube around a diagonal axis and a sphere around the Y axis. The rotation speed is controlled by the elapsed time.

## animation2 Example

The `animation2` example shows a more complex Lissajous animation with an interactive UI.

### Key Concepts Demonstrated

- Lissajous curve animation
- LineTrace for path visualization
- Interactive parameter control with ImGui
- Dynamic parameter updates

### Lissajous Class

```cpp
class Lissajous {
private:
    float m_a, m_b, m_c, m_d, m_e, m_f, m_g, m_h, m_i;

public:
    // ... constructor and methods ...

    glm::vec3 operator()(float t) {
        return glm::vec3(m_a * sin(m_b * t + m_d), m_c * sin(m_e * t + m_f), m_g * sin(m_h * t + m_i));
    }
};
```

### Update Loop

```cpp
virtual void onUpdate() override
{
    // Check if UI parameters were changed
    if (m_lissajouWindow->is_dirty())
    {
        float a, b, c, d, e, f, g, h, i;
        m_lissajouWindow->get_params(a, b, c, d, e, f, g, h, i);
        m_lissajous.setParameters(a, b, c, d, e, f, g, h, i);
        m_speed = m_lissajouWindow->speed();
        m_trace->setSize(m_lissajouWindow->size());
        m_trace->reset();
    }

    // Update sphere position
    auto pos = m_lissajous(elapsedTime() * m_speed);
    m_sphere->setPos(pos);
    m_trace->add(pos);
    m_trace->refresh();
}
```
```

### ui/ui_windows.md

```markdown
# UI Windows

ivf2 provides a user interface system built on Dear ImGui.

## Overview

The UI window system in ivf2 allows you to create customizable user interface elements for controlling your application. UI windows are implemented using the Dear ImGui library, providing a lightweight and immediate-mode interface.

## Creating a UI Window

To create a UI window, subclass the `ivfui::UiWindow` class:

```cpp
class MyControlWindow : public ivfui::UiWindow {
private:
    float m_value;
    bool m_toggle;
    
public:
    MyControlWindow() : UiWindow("My Controls"), m_value(0.0f), m_toggle(false) {}
    
    static std::shared_ptr<MyControlWindow> create() {
        return std::make_shared<MyControlWindow>();
    }
    
    void doDraw() override {
        ImGui::Text("Control Panel");
        ImGui::SliderFloat("Value", &m_value, 0.0f, 1.0f);
        ImGui::Checkbox("Toggle", &m_toggle);
        
        if (ImGui::Button("Reset")) {
            m_value = 0.0f;
            m_toggle = false;
        }
    }
    
    float getValue() const { return m_value; }
    bool getToggle() const { return m_toggle; }
};
```

## Adding UI Windows to a Scene

To add a UI window to your scene:

```cpp
class MyApplication : public ivfui::GLFWSceneWindow {
private:
    MyControlWindowPtr m_controlWindow;
    
public:
    int onSetup() override {
        // Create UI window
        m_controlWindow = MyControlWindow::create();
        
        // Add UI window to the scene
        this->addUiWindow(m_controlWindow);
        
        return 0;
    }
    
    void onUpdateUi() override {
        // Use the UI values in your scene
        if (m_controlWindow->getToggle()) {
            // Do something based on toggle state
        }
        
        float value = m_controlWindow->getValue();
        // Use the value
    }
};
```

## UI Window Features

- **Show/Hide**: Control window visibility with `show()` and `hide()` methods
- **Window title**: Set in the constructor
- **State access**: Create accessors to get/set internal state
- **ImGui widgets**: Use any ImGui widget in your `doDraw()` method
```
