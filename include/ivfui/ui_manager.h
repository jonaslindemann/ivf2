#pragma once

/**
 * @file ui_manager.h
 * @brief Declares the UiRenderer class for ImGui-based UI rendering in the ivfui library.
 */

#include <imgui.h>
#include <GLFW/glfw3.h>
#include <memory>

namespace ivfui {

/**
 * @class UiRenderer
 * @brief Manages ImGui UI rendering for a GLFW window.
 *
 * The UiRenderer class encapsulates the setup, frame management, and rendering
 * of ImGui-based user interfaces for a given GLFW window. It provides methods
 * for frame lifecycle control, drawing, and shutdown, as well as input capture queries.
 */
class UiRenderer {
private:
public:
    /**
     * @brief Construct a UiRenderer for the given GLFW window.
     * @param window Pointer to the GLFW window.
     */
    UiRenderer(GLFWwindow *window);

    /**
     * @brief Destructor. Cleans up ImGui and rendering resources.
     */
    virtual ~UiRenderer();

    /**
     * @brief Factory method to create a shared pointer to a UiRenderer instance.
     * @param window Pointer to the GLFW window.
     * @return std::shared_ptr<UiRenderer> New UiRenderer instance.
     */
    static std::shared_ptr<UiRenderer> create(GLFWwindow *window);

    /**
     * @brief Begin a new ImGui frame (should be called at the start of each frame).
     */
    void beginFrame();

    /**
     * @brief End the current ImGui frame (should be called at the end of each frame).
     */
    void endFrame();

    /**
     * @brief Render the ImGui UI for the current frame.
     */
    void draw();

    /**
     * @brief Shutdown the UI renderer and release resources.
     */
    void shutdown();

    /**
     * @brief Check if ImGui wants to capture mouse input.
     * @return bool True if ImGui wants to capture mouse input.
     */
    bool wantCaptureMouse();

    /**
     * @brief Check if ImGui wants to capture keyboard input.
     * @return bool True if ImGui wants to capture keyboard input.
     */
    bool wantCaptureKeyboard();

    void scaleAllSizes(float scale);
};

/**
 * @typedef UiRendererPtr
 * @brief Shared pointer type for UiRenderer.
 */
typedef std::shared_ptr<UiRenderer> UiRendererPtr;

} // namespace ivfui
