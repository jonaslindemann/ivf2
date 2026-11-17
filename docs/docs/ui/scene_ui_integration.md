# Built-in Scene Tools

Every `GLFWSceneWindow` comes with a set of ready-made tools: a main menu, panels for the camera, scene and effects, and keyboard shortcuts to open them. You get all of this without writing any UI code. This page describes what is there and how to extend it.

## Built-in panels

The window creates these panels at startup, hidden. Open them with a key, from the main menu, or from code:

| Panel | Key | From code | What it does |
|-------|-----|-----------|--------------|
| Camera | **V** | `showCameraWindow()` | Camera position, target and projection, with zoom to extent |
| Scene Inspector | **S** | `showSceneInspector()` | Tree of every node in the scene; select a node to see and edit its properties |
| Effect Inspector | **E** | `showEffectInspector()` | Every [post-processing effect](../effects/index.md): enable, reorder and tweak |
| Scene Control Panel | | `showControlPanel()` | Axis and grid visibility, axis length, grid ticks and spacing |
| Grid snap dialog | **Ctrl+G** in the menu | `showGridSnapDialog()` | Value that the 3D cursor snaps to |

Other keys toggle scene helpers directly:

| Key | Toggles |
|-----|---------|
| **A** | Axis |
| **G** | Grid |
| **C** | 3D cursor |

The panels are meant for development: they let you explore and tune a scene while it runs. Once you have found good values, write them into your code.

## The main menu

The main menu bar has a **File** menu and a **View** menu. The View menu contains zoom to extent, reset and save view, the panels above, and toggles for the headlight, axis, grid and cursor.

Hide or show the whole bar with `hideMainMenu()` / `showMainMenu()`, for example for a clean presentation.

### Adding your own menu items

Override `onAddMenuItems()`. It is called once for each built-in menu while the menu bar is being built, so check the menu name:

```cpp
void onAddMenuItems(ivfui::UiMenu *menu) override
{
    if (menu->name() == "File")
    {
        menu->addItem(ivfui::UiMenuItem::create("Open...", "CTRL+O", [this]() { onOpen(); }));
        menu->addSeparator();
    }
    if (menu->name() == "View")
    {
        menu->addItem(ivfui::UiMenuItem::create(
            "Wireframe", "",
            [this]() { m_wireframe = !m_wireframe; },   // action
            [this]() { return m_wireframe; }));          // shows a check mark when true
    }
}
```

The shortcut text is only displayed in the menu; handle the key itself in `onKey()`.

## Inspecting your own objects

The Scene Inspector and Effect Inspector show properties of objects that implement `PropertyInspectable`. All nodes, effects and deformers already do. To inspect one particular object in its own window, use an `ObjectInspector`:

```cpp
auto inspector = ivfui::ObjectInspector::create("Twist");
inspector->setObject(m_twistDeformer);
inspector->setVisible(true);
addUiWindow(inspector);
```

React to edits with a callback, for example to reapply deformers only when something changed:

```cpp
inspector->setPropertyChangedCallback([this](const std::string &name) {
    m_shape->applyDeformers();
});
```

To make your own classes inspectable, see [Custom UI](custom_ui.md#making-your-classes-inspectable).

## Reacting to the scene inspector

To find out which node the user picked in the Scene Inspector, create your own inspector and add a selection callback:

```cpp
auto inspector = ivfui::SceneInspector::create("Objects");
inspector->setRootNode(scene());
inspector->setOnNodeSelected([this](std::shared_ptr<ivf::Node> node) {
    m_selected = node;
});
addUiWindow(inspector);
```

`setShowInvisibleNodes()`, `setShowNodeTypes()` and `setShowObjectIds()` control what the tree shows.

## Related

- [UI Windows](ui_windows.md) — creating your own panels
- [Ready-made Widgets](widgets.md)
- [Scene Window](scene_window.md)
