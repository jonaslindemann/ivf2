#pragma once

#include <ivf/mesh_node.h>

namespace ivf {

/**
 * @class LineTrace
 * @brief Node representing a dynamic polyline (trace) with configurable color and vertex count.
 *
 * The LineTrace class provides a polyline (trace) that can be dynamically updated by adding or
 * modifying vertices. It supports color configuration, resizing, and efficient refresh for
 * real-time visualization or path tracing. Inherits from MeshNode, allowing transformation and
 * integration into a scene graph.
 */
class LineTrace : public MeshNode {
private:
    int m_numVertices;  ///< Number of vertices in the trace.
    bool m_useColor;    ///< Whether to use a custom color.
    GLfloat m_color[4]; ///< RGBA color for the trace.
    bool m_firstAdd;    ///< Internal flag for first vertex addition.

public:
    /**
     * @brief Constructor.
     * @param numVertices Number of vertices in the trace.
     */
    LineTrace(int numVertices);

    /**
     * @brief Destructor.
     */
    virtual ~LineTrace();

    /**
     * @brief Factory method to create a shared pointer to a LineTrace instance.
     * @param numVertices Number of vertices in the trace.
     * @return std::shared_ptr<LineTrace> New LineTrace instance.
     */
    static std::shared_ptr<LineTrace> create(int numVertices);

    /**
     * @brief Set the color of the trace.
     * @param r Red component.
     * @param g Green component.
     * @param b Blue component.
     * @param a Alpha component (default 1.0).
     */
    void setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0);

    /**
     * @brief Enable or disable the use of a custom color.
     * @param value True to use custom color, false to use default.
     */
    void setUseColor(bool value);

    /**
     * @brief Check if a custom color is used.
     * @return bool True if custom color is used.
     */
    bool useColor() const;

    /**
     * @brief Get the current color of the trace.
     * @param r Reference to store red component.
     * @param g Reference to store green component.
     * @param b Reference to store blue component.
     * @param a Reference to store alpha component.
     */
    void getColor(GLfloat &r, GLfloat &g, GLfloat &b, GLfloat &a);

    /**
     * @brief Set the number of vertices in the trace.
     * @param numVertices Number of vertices.
     */
    void setNumVertices(int numVertices);

    /**
     * @brief Get the number of vertices in the trace.
     * @return int Number of vertices.
     */
    int numVertices() const;

    /**
     * @brief Set the position of a vertex in the trace.
     * @param idx Index of the vertex.
     * @param vertex Position of the vertex (glm::vec3).
     */
    void setVertex(int idx, glm::vec3 &vertex);

    /**
     * @brief Reset the trace to its initial state.
     */
    void reset();

    /**
     * @brief Start a new trace with the given vertex.
     * @param vertex First vertex of the trace.
     */
    void start(glm::vec3 vertex);

    /**
     * @brief Add a vertex to the trace.
     * @param vertex Vertex to add.
     */
    void add(glm::vec3 vertex);

    /**
     * @brief Set the size (number of vertices) of the trace.
     * @param size Number of vertices.
     */
    void setSize(size_t size);

    /**
     * @brief Refresh the trace geometry (update mesh data).
     */
    void refresh();

protected:
    /**
     * @brief Internal setup method for initializing the trace geometry.
     */
    virtual void doSetup();

    /**
     * @brief Called before drawing the trace node.
     */
    virtual void doPreDraw();

    /**
     * @brief Called after drawing the trace node.
     */
    virtual void doPostDraw();
};

/**
 * @typedef LineTracePtr
 * @brief Shared pointer type for LineTrace.
 */
typedef std::shared_ptr<LineTrace> LineTracePtr;

}; // namespace ivf
