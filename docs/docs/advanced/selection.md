# Selection

Selection tells you which object is under the mouse cursor, so the user can point at things: highlight what they hover over, click to pick an object, or drag a rectangle to select several.

![Selection](../images/sc_selection1.png)

## How it works

When selection is on, the window renders the scene a second time each frame into a hidden buffer, drawing every object in a unique flat colour that encodes its ID. Reading the pixel under the cursor tells the window exactly which object is there. This is pixel-accurate, also for complex and overlapping shapes, and costs one extra render of the scene per frame.

## Hover events

Turn selection on in `onSetup()` and override the three hover callbacks:

```cpp
class ExampleWindow : public ivfui::GLFWSceneWindow {
    ivf::MaterialPtr m_normalMat, m_hoverMat;

    int onSetup() override
    {
        setSelectionEnabled(true);
        // ... create materials and objects ...
        return 0;
    }

    void onEnterNode(ivf::Node *node) override
    {
        node->setMaterial(m_hoverMat);    // cursor moved onto this node
    }

    void onOverNode(ivf::Node *node) override
    {
        // called every frame while the cursor stays on the node
    }

    void onLeaveNode(ivf::Node *node) override
    {
        node->setMaterial(m_normalMat);   // cursor left this node
    }
};
```

The callbacks receive a raw `Node*`. It stays valid as long as the node is in the scene, but don't store it beyond that; keep the `shared_ptr` if you need the object later.

Only the frontmost object under the cursor is reported.

## Clicking to pick

Combine the hover events with `onMouseButton()`: remember the hovered node, and act on it when the button is pressed.

```cpp
ivf::Node *m_hovered{nullptr};
ivf::Node *m_selected{nullptr};

void onEnterNode(ivf::Node *node) override { m_hovered = node; }
void onLeaveNode(ivf::Node *node) override { if (m_hovered == node) m_hovered = nullptr; }

void onMouseButton(int button, int action, int mods) override
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && m_hovered)
    {
        if (m_selected)
            m_selected->setMaterial(m_normalMat);
        m_selected = m_hovered;
        m_selected->setMaterial(m_selectedMat);
    }
}
```

## Box selection

To select several objects at once by dragging a rectangle, enable box selection and override `onBoxSelect()`:

```cpp
int onSetup() override
{
    setSelectionEnabled(true);
    setBoxSelectionEnabled(true);
    return 0;
}

void onBoxSelect(std::vector<ivf::Node *> nodes) override
{
    for (auto node : nodes)
        node->setMaterial(m_selectedMat);
}
```

A drag shorter than a few pixels counts as a click and doesn't trigger a box selection. Only objects that are visible inside the rectangle are returned; objects fully hidden behind others are not.

## The 3D cursor position

`onMousePosition3D(x, y, z)` reports the point in the scene under the mouse cursor, which is useful for placing objects where the user points. Together with grid snapping (`setSnapToGrid(true)`, `setGridSnapValue()`) and the built-in 3D cursor (`enableCursor()`), this is the basis for simple editors. See the `editor1` example.

## Performance

The selection pass renders the whole scene again every frame. With very large scenes, enable selection only while the user is in a picking mode, and turn it off with `setSelectionEnabled(false)` afterwards.

## Related

- [Interaction Examples](../examples/interaction_examples.md) — the `selection1` example
- [Scene Window](../ui/scene_window.md)
