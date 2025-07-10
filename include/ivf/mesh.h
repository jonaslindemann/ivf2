#pragma once

#include <ivf/glbase.h>
#include <ivf/vertices.h>
#include <ivf/colors.h>
#include <ivf/normals.h>
#include <ivf/indices.h>
#include <ivf/tex_coords.h>
#include <ivf/material.h>

#include <ivf/vertex_array.h>
#include <ivf/vertex_buffer.h>
#include <ivf/index_buffer.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec3.hpp>

#include <memory>

namespace ivf {

/**
 * @class Mesh
 * @brief Represents a 3D mesh with vertex, color, normal, index, and texture coordinate data.
 *
 * The Mesh class encapsulates the data and operations for a 3D mesh, including vertex positions,
 * colors, normals, indices, and texture coordinates. It manages OpenGL buffers and supports
 * drawing, updating, and configuring mesh attributes for rendering.
 */
class Mesh : public GLBase {
private:
    GLuint m_primType; ///< OpenGL primitive type (e.g., GL_TRIANGLES).

    std::shared_ptr<Vertices> m_verts;      ///< Vertex positions.
    std::shared_ptr<Colors> m_colors;       ///< Vertex colors.
    std::shared_ptr<Indices> m_indices;     ///< Index data.
    std::shared_ptr<Normals> m_normals;     ///< Vertex normals.
    std::shared_ptr<TexCoords> m_texCoords; ///< Texture coordinates.

    std::shared_ptr<Vertices> m_glVerts;  ///< OpenGL-side vertex positions.
    std::shared_ptr<Colors> m_glColors;   ///< OpenGL-side vertex colors.
    std::shared_ptr<Normals> m_glNormals; ///< OpenGL-side vertex normals.

    ivf::MaterialPtr m_material; ///< Material properties for the mesh.

    int m_vertPos;     ///< Current vertex position index.
    int m_colorPos;    ///< Current color position index.
    int m_normalPos;   ///< Current normal position index.
    int m_indexPos;    ///< Current index position.
    int m_texCoordPos; ///< Current texture coordinate position.

    int m_indexSize; ///< Number of indices.

    GLint m_vertexAttrId;   ///< Shader attribute location for vertex position.
    GLint m_colorAttrId;    ///< Shader attribute location for color.
    GLint m_normalAttrId;   ///< Shader attribute location for normal.
    GLint m_texCoordAttrId; ///< Shader attribute location for texture coordinate.

    bool m_wireframe{false}; ///< Render mesh as wireframe.

    std::unique_ptr<VertexArray> m_VAO;          ///< Vertex array object.
    std::unique_ptr<VertexBuffer> m_vertexVBO;   ///< Vertex buffer object for positions.
    std::unique_ptr<VertexBuffer> m_colorVBO;    ///< Vertex buffer object for colors.
    std::unique_ptr<VertexBuffer> m_normalVBO;   ///< Vertex buffer object for normals.
    std::unique_ptr<IndexBuffer> m_indexVBO;     ///< Index buffer object.
    std::unique_ptr<VertexBuffer> m_texCoordVBO; ///< Vertex buffer object for texture coordinates.

    glm::vec3 m_position; ///< Mesh position in world space.

    bool m_generateNormals; ///< Whether to generate normals automatically.
    bool m_enabled;         ///< Whether the mesh is enabled for rendering.

    float m_polygonOffsetFactor;    ///< Polygon offset factor (for depth bias).
    float m_polygonOffsetUnits;     ///< Polygon offset units (for depth bias).
    GLenum m_depthFunc;             ///< Depth test function.
    GLfloat m_lineWidth;            ///< Line width for wireframe rendering.
    GLenum m_usage{GL_STATIC_DRAW}; ///< OpenGL buffer usage hint.

    /**
     * @brief Internal method to set up the OpenGL primitive type.
     */
    void setupPrim();

public:
    /**
     * @brief Constructor.
     * @param vsize Number of vertices.
     * @param isize Number of indices (default 0).
     * @param primType OpenGL primitive type (default GL_TRIANGLES).
     * @param usage OpenGL buffer usage (default GL_STATIC_DRAW).
     */
    Mesh(GLuint vsize, GLuint isize = 0, GLuint primType = GL_TRIANGLES, GLenum usage = GL_STATIC_DRAW);

    /**
     * @brief Destructor.
     */
    virtual ~Mesh();

    /**
     * @brief Factory method to create a shared pointer to a Mesh instance.
     * @param vsize Number of vertices.
     * @param isize Number of indices (default 0).
     * @return std::shared_ptr<Mesh> New Mesh instance.
     */
    static std::shared_ptr<Mesh> create(GLuint vsize, GLuint isize = 0);

    /**
     * @brief Set the number of vertices and indices.
     * @param vsize Number of vertices.
     * @param isize Number of indices (default 0).
     */
    void setSize(GLuint vsize, GLuint isize = 0);

    /**
     * @brief Enable or disable the mesh for rendering.
     * @param flag True to enable, false to disable.
     */
    void setEnabled(bool flag);

    /**
     * @brief Check if the mesh is enabled for rendering.
     * @return bool True if enabled.
     */
    bool enabled();

    /**
     * @brief Enable or disable automatic normal generation.
     * @param flag True to enable, false to disable.
     */
    void setGenerateNormals(bool flag);

    /**
     * @brief Check if automatic normal generation is enabled.
     * @return bool True if enabled.
     */
    bool generateNormals();

    /**
     * @brief Set the polygon offset for depth bias.
     * @param factor Offset factor.
     * @param units Offset units.
     */
    void setPolygonOffset(float factor, float units);

    /**
     * @brief Get the polygon offset factor.
     * @return float Offset factor.
     */
    float polygonOffsetFactor();

    /**
     * @brief Get the polygon offset units.
     * @return float Offset units.
     */
    float polygonOffsetUnits();

    /**
     * @brief Set the OpenGL depth test function.
     * @param func Depth function (e.g., GL_LESS).
     */
    void setDepthFunc(GLenum func);

    /**
     * @brief Get the OpenGL depth test function.
     * @return GLenum Depth function.
     */
    GLenum depthFunc();

    /**
     * @brief Set the line width for wireframe rendering.
     * @param width Line width.
     */
    void setLineWidth(GLfloat width);

    /**
     * @brief Get the line width for wireframe rendering.
     * @return GLfloat Line width.
     */
    GLfloat lineWidth();

    /**
     * @brief Enable or disable wireframe rendering.
     * @param flag True for wireframe, false for filled.
     */
    void setWireframe(bool flag);

    /**
     * @brief Check if wireframe rendering is enabled.
     * @return bool True if wireframe is enabled.
     */
    bool wireframe();

    /**
     * @brief Set the shader attribute location for vertex position.
     * @param id Attribute location.
     */
    void setVertexAttrId(GLuint id);

    /**
     * @brief Set the shader attribute location for color.
     * @param id Attribute location.
     */
    void setColorAttrId(GLuint id);

    /**
     * @brief Set the shader attribute location for normal.
     * @param id Attribute location.
     */
    void setNormalAttrId(GLuint id);

    /**
     * @brief Set the shader attribute location for texture coordinate.
     * @param id Attribute location.
     */
    void setTexCoordAttrId(GLuint id);

    /**
     * @brief Begin mesh definition with a specific primitive type.
     * @param primType OpenGL primitive type.
     */
    void begin(GLuint primType);

    /**
     * @brief Add a vertex (float) to the mesh.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param z Z coordinate.
     */
    void vertex3f(GLfloat x, GLfloat y, GLfloat z);

    /**
     * @brief Add a vertex (double) to the mesh.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param z Z coordinate.
     */
    void vertex3d(GLdouble x, GLdouble y, GLdouble z);

    /**
     * @brief Add a 2D vertex (float) to the mesh.
     * @param x X coordinate.
     * @param y Y coordinate.
     */
    void vertex2f(GLfloat x, GLfloat y);

    /**
     * @brief Add a 2D vertex (double) to the mesh.
     * @param x X coordinate.
     * @param y Y coordinate.
     */
    void vertex2d(GLdouble x, GLdouble y);

    /**
     * @brief Add a vertex from a glm::dvec3.
     * @param v 3D vector.
     */
    void vertex3d(const glm::dvec3 &v);

    /**
     * @brief Add a vertex from a glm::vec3.
     * @param v 3D vector.
     */
    void vertex3f(const glm::vec3 v);

    /**
     * @brief Add a texture coordinate.
     * @param s S (U) coordinate.
     * @param t T (V) coordinate.
     */
    void tex2f(GLfloat s, GLfloat t);

    /**
     * @brief Add a normal (float) to the mesh.
     * @param vx X component.
     * @param vy Y component.
     * @param vz Z component.
     */
    void normal3f(GLfloat vx, GLfloat vy, GLfloat vz);

    /**
     * @brief Add a normal from a glm::vec3.
     * @param v Normal vector.
     */
    void normal3f(const glm::vec3 v);

    /**
     * @brief Add a normal (double) to the mesh.
     * @param vx X component.
     * @param vy Y component.
     * @param vz Z component.
     */
    void normal3d(GLdouble vx, GLdouble vy, GLdouble vz);

    /**
     * @brief Set the color for the next vertex (float, RGB).
     * @param r Red component.
     * @param g Green component.
     * @param b Blue component.
     */
    void color3f(GLfloat r, GLfloat g, GLfloat b);

    /**
     * @brief Set the color for the next vertex (float, RGBA).
     * @param r Red component.
     * @param g Green component.
     * @param b Blue component.
     * @param a Alpha component.
     */
    void color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);

    /**
     * @brief Add a single index.
     * @param i0 Index value.
     */
    void index1i(GLuint i0);

    /**
     * @brief Add two indices.
     * @param i0 First index.
     * @param i1 Second index.
     */
    void index2i(GLuint i0, GLuint i1);

    /**
     * @brief Add three indices.
     * @param i0 First index.
     * @param i1 Second index.
     * @param i2 Third index.
     */
    void index3i(GLuint i0, GLuint i1, GLuint i2);

    /**
     * @brief Add four indices.
     * @param i0 First index.
     * @param i1 Second index.
     * @param i2 Third index.
     * @param i3 Fourth index.
     */
    void index4i(GLuint i0, GLuint i1, GLuint i2, GLuint i3);

    /**
     * @brief Add a quad as two triangles.
     * @param w Width of the quad.
     * @param h Height of the quad.
     * @param offset Offset along the normal.
     * @param vx X component of the normal.
     * @param vy Y component of the normal.
     * @param vz Z component of the normal.
     */
    void triQuad(GLdouble w, GLdouble h, GLdouble offset, GLdouble vx, GLdouble vy, GLdouble vz);

    /**
     * @brief End mesh definition.
     */
    void end();

    /**
     * @brief Update the vertex buffer with new vertex data.
     */
    void updateVertices();

    /**
     * @brief Update the normal buffer with new normal data.
     */
    void updateNormals();

    /**
     * @brief Draw the mesh using the current OpenGL state.
     */
    void draw();

    /**
     * @brief Draw the mesh using a specific OpenGL primitive type.
     * @param prim OpenGL primitive type.
     */
    void drawAsPrim(GLuint prim);

    /**
     * @brief Set the mesh position in world space.
     * @param pos Position vector.
     */
    void setPos(glm::vec3 pos);

    /**
     * @brief Get the mesh position in world space.
     * @return glm::vec3 Position vector.
     */
    glm::vec3 pos();

    /**
     * @brief Get the current vertex position index.
     * @return int Vertex position index.
     */
    int vertPos() const;

    /**
     * @brief Get the current color position index.
     * @return int Color position index.
     */
    int colorPos() const;

    /**
     * @brief Get the current normal position index.
     * @return int Normal position index.
     */
    int normalPos() const;

    /**
     * @brief Get the current index position.
     * @return int Index position.
     */
    int indexPos() const;

    /**
     * @brief Get the current texture coordinate position.
     * @return int Texture coordinate position.
     */
    int texCoordPos() const;

    /**
     * @brief Get the current index buffer position.
     * @return GLuint Index buffer position.
     */
    GLuint currentIndexPos();

    /**
     * @brief Get the normals array.
     * @return std::shared_ptr<Normals> Normals array.
     */
    std::shared_ptr<Normals> normals();

    /**
     * @brief Get the vertices array.
     * @return std::shared_ptr<Vertices> Vertices array.
     */
    std::shared_ptr<Vertices> vertices();

    /**
     * @brief Get the indices array.
     * @return std::shared_ptr<Indices> Indices array.
     */
    std::shared_ptr<Indices> indices();

    ivf::MaterialPtr material() const;
    void setMaterial(ivf::MaterialPtr material);
    /**
     * @brief Print mesh information for debugging.
     */
    void print();
};

/**
 * @typedef MeshPtr
 * @brief Shared pointer type for Mesh.
 */
typedef std::shared_ptr<Mesh> MeshPtr;

}; // namespace ivf
