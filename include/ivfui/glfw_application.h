#pragma once

/**
 * @file glfw_application.h
 * @brief Declares the GLFWApplication and GLFWWindowTracker classes for managing GLFW-based applications and windows in
 * the ivfui library.
 */

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <memory>
#include <string>
#include <vector>

#include "glfw_window.h"

namespace ivfui {

/**
 * @class GLFWApplication
 * @brief Manages the main application loop and multiple GLFW windows.
 *
 * The GLFWApplication class provides a framework for creating and managing
 * multiple GLFW windows, handling the main event loop, and integrating with
 * the ivfui windowing system. It supports adding windows, polling events,
 * and setting GLFW window hints.
 */
class GLFWApplication {
private:
    std::vector<GLFWWindowPtr> m_windows; ///< List of managed GLFW windows.

public:
    /**
     * @brief Default constructor. Initializes the application.
     */
    GLFWApplication();

    /**
     * @brief Destructor. Cleans up application resources.
     */
    virtual ~GLFWApplication();

    /**
     * @brief Factory method to create a shared pointer to a GLFWApplication instance.
     * @return std::shared_ptr<GLFWApplication> New GLFWApplication instance.
     */
    static std::shared_ptr<GLFWApplication> create();

    /**
     * @brief Add a window to the application.
     * @param window Shared pointer to the GLFWWindow to add.
     */
    void addWindow(GLFWWindowPtr window);

    /**
     * @brief Run the main application loop.
     * @return int Exit code.
     */
    int loop();

    /**
     * @brief Poll for windowing and input events.
     */
    void pollEvents();

    /**
     * @brief Set a GLFW window hint.
     * @param hint GLFW hint constant.
     * @param value Value for the hint.
     */
    void hint(int hint, int value);
};

/**
 * @typedef GLFWApplicationPtr
 * @brief Shared pointer type for GLFWApplication.
 */
typedef std::shared_ptr<GLFWApplication> GLFWApplicationPtr;

/**
 * @class GLFWWindowTracker
 * @brief Singleton for tracking and mapping GLFWwindow pointers to GLFWWindowPtr objects.
 *
 * The GLFWWindowTracker class maintains a mapping between raw GLFWwindow pointers and
 * their corresponding GLFWWindowPtr objects. This enables efficient lookup and management
 * of window resources in callback scenarios.
 */
class GLFWWindowTracker {
private:
    std::map<GLFWwindow *, GLFWWindowPtr> m_windowMap; ///< Map of GLFWwindow* to GLFWWindowPtr.

    /**
     * @brief Private constructor for singleton pattern.
     */
    GLFWWindowTracker(){};

public:
    /**
     * @brief Get the singleton instance of the window tracker.
     * @return GLFWWindowTracker* Pointer to the singleton instance.
     */
    static GLFWWindowTracker *instance()
    {
        static GLFWWindowTracker m_instance;
        return &m_instance;
    }

    /**
     * @brief Add a window to the tracker.
     * @param window Shared pointer to the GLFWWindow to add.
     */
    void addWindow(GLFWWindowPtr window);

    /**
     * @brief Get the GLFWWindowPtr associated with a raw GLFWwindow pointer.
     * @param window Raw GLFWwindow pointer.
     * @return GLFWWindowPtr Shared pointer to the GLFWWindow.
     */
    GLFWWindowPtr get(GLFWwindow *window);
};

}; // namespace ivfui
