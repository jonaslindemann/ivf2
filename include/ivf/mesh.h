#pragma once

#include <ivf/glbase.h>
#include <ivf/vertices.h>
#include <ivf/colors.h>
#include <ivf/normals.h>
#include <ivf/indices.h>
#include <ivf/tex_coords.h>
#include <ivf/material.h>
#include <ivf/mesh_bvh.h>

#include <ivf/vertex_array.h>
#include <ivf/vertex_buffer.h>
#include <ivf/index_buffer.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec3.hpp>

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace ivf {

class RenderContext;

/**
 * @class Mesh
 * @brief Represents a 3D mesh with vertex, color, normal, index, and texture coordinate data.
 *
 * The Mesh class encapsulates the data and operations for a 3D mesh, including vertex positions,
 * colors, normals, indices, and texture coordinates.
 *
 * Building a mesh (begin()/vertex3f()/.../end(), updateVertices(), updateNormals()) only
 * changes CPU-side data and needs no OpenGL context. The data is uploaded when the mesh is
 * first drawn: each RenderContext (OpenGL context) that draws the mesh gets its own GPU copy,
 * brought up to date before drawing, so a mesh can be shown in several windows. A copy is
 * released with the mesh or its context.
 *
 * Vertex attributes use the fixed locations every ShaderManager program binds:
 * aPos = 0, aColor = 1, aTex = 2, aNormal = 3.
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

    /// GPU buffers of the mesh in one OpenGL context.
    struct GpuCopy {
        RenderContext *context{nullptr};      ///< Context the objects belong to.
        std::weak_ptr<const void> lifetime;   ///< Expires with the context (RenderContext::lifetime()).
        std::unique_ptr<VertexArray> vao;     ///< Vertex array object.
        std::unique_ptr<VertexBuffer> vertices, colors, normals, texCoords; ///< Attribute buffers.
        std::unique_ptr<IndexBuffer> indices; ///< Index buffer.
        std::uint64_t dataVersion{0};         ///< m_dataVersion when uploaded.
        std::uint64_t vertexVersion{0};       ///< m_vertexVersion when uploaded.
        std::uint64_t normalVersion{0};       ///< m_normalVersion when uploaded.
        std::uint64_t colorVersion{0};        ///< m_colorVersion when uploaded.
        GLuint wireIndices{0};                ///< Triangle edges as GL_LINES (wireframe on OpenGL ES).
        GLsizei wireCount{0};                 ///< Number of indices in wireIndices.
        std::uint64_t wireVersion{0};         ///< m_dataVersion when wireIndices was built.
};
    std::vector<std::unique_ptr<GpuCopy>> m_gpu; ///< One GPU copy per context that drew the mesh.

    std::uint64_t m_dataVersion{0};   ///< Incremented by end(); 0 = never ended, nothing to draw.
    std::uint64_t m_vertexVersion{0}; ///< Incremented by updateVertices().
    MeshBvhPtr m_bvh;                           ///< See bvh().
    std::uint64_t m_bvhDataVersion{0};          ///< m_dataVersion when m_bvh was built.
    std::uint64_t m_bvhVertexVersion{0};        ///< m_vertexVersion when m_bvh was built.
    std::uint64_t m_normalVersion{0}; ///< Incremented by updateNormals().
    std::uint64_t m_colorVersion{0};  ///< Incremented by updateColors().
bool m_clockwise{false};          ///< Front faces are clockwise (decided in end()).

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

    /**
     * @brief Draw the mesh, instanced when @p instanceCount is greater than 0.
     */
    void drawImpl(RenderContext &context, GLsizei instanceCount, std::span<const glm::mat4> transforms = {});

    /// GPU copy for @p context, created or updated as needed (nullptr before end()).
    GpuCopy *gpuCopy(RenderContext &context);
    /// Create the buffers of @p copy from the CPU data (the copy's context is current).
    void upload(GpuCopy &copy);
    /// Release the GL objects of @p copy in the right context.
    static void release(GpuCopy &copy);
    /// Edges of the triangles as a GL_LINES index buffer (built on first use, per data version).
    void updateWireIndices(GpuCopy &copy);

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
    [[nodiscard]] static std::shared_ptr<Mesh> create(GLuint vsize, GLuint isize = 0);

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
    bool enabled() const;

    /**
     * @brief Enable or disable automatic normal generation.
     * @param flag True to enable, false to disable.
     */
    void setGenerateNormals(bool flag);

    /**
     * @brief Check if automatic normal generation is enabled.
     * @return bool True if enabled.
     */
    bool generateNormals() const;

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
    float polygonOffsetFactor() const;

    /**
     * @brief Get the polygon offset units.
     * @return float Offset units.
     */
    float polygonOffsetUnits() const;

    /**
     * @brief Set the OpenGL depth test function.
     * @param func Depth function (e.g., GL_LESS).
     */
    void setDepthFunc(GLenum func);

    /**
     * @brief Get the OpenGL depth test function.
     * @return GLenum Depth function.
     */
    GLenum depthFunc() const;

    /**
     * @brief Set the line width for wireframe rendering.
     * @param width Line width.
     */
    void setLineWidth(GLfloat width);

    /**
     * @brief Get the line width for wireframe rendering.
     * @return GLfloat Line width.
     */
    GLfloat lineWidth() const;

    /**
     * @brief Enable or disable wireframe rendering.
     * @param flag True for wireframe, false for filled.
     */
    void setWireframe(bool flag);

    /**
     * @brief Check if wireframe rendering is enabled.
     * @return bool True if wireframe is enabled.
     */
    bool wireframe() const;

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
     * @brief End mesh definition (generates normals if enabled).
     *
     * Needs no OpenGL context: the data is uploaded before the mesh is next drawn.
     */
    void end();

    /**
     * @brief True if the mesh's front faces (those its normals point away from) are wound
     * clockwise; drawing then sets glFrontFace(GL_CW) so face culling removes the right side.
     */
    [[nodiscard]] bool clockwise() const
    {
        return m_clockwise;
    }

    /**
     * @brief Mark the vertex positions as changed; they are uploaded before the next draw.
     */
    void updateVertices();

    /**
     * @brief Recompute normals from the vertex positions; uploaded before the next draw.
     */
    void updateNormals();

    /**
     * @brief Mark the vertex colors (colors()) as changed; they are uploaded before the next draw.
     */
    void updateColors();

    /**
     * @brief Draw the mesh using the current OpenGL state, in the current RenderContext.
     */
    void draw();

    /**
     * @brief Draw the mesh into @p context (its OpenGL context must be current).
     *
     * Uses (and if needed creates or updates) the mesh's GPU copy for that context.
     * @param context Render context to draw into.
     */
    void draw(RenderContext &context);

    /**
     * @brief Draw @p instanceCount instances of the mesh in one draw call.
     *
     * Per-instance vertex attributes must be attached to the mesh's vertex
     * array beforehand (see vertexArray() and InstancedMeshNode).
     * @param instanceCount Number of instances to draw.
     */
    void drawInstanced(GLsizei instanceCount);

    /**
     * @brief Draw @p instanceCount instances of the mesh into @p context.
     * @param context Render context to draw into (its OpenGL context must be current).
     * @param instanceCount Number of instances to draw.
     */
    void drawInstanced(RenderContext &context, GLsizei instanceCount);

    /**
     * @brief Draw the mesh once per model matrix in @p transforms (automatic instancing).
     *
     * Applies the mesh's material, then draws all copies with one instanced draw call if
     * the current program has the `useInstancing` uniform (stock and PBR shaders; see
     * ivf/instancing.h), or one draw per copy otherwise. The model matrix of the
     * TransformManager is restored afterwards.
     * @param context Render context to draw into (its OpenGL context must be current).
     * @param transforms World (model) matrix of each copy.
     */
    void drawInstances(RenderContext &context, std::span<const glm::mat4> transforms);

    /**
     * @brief Get the vertex array object of the mesh in the current context.
     *
     * Uploads the mesh to the current context first if needed. Each context has its own
     * vertex array, and a new one is created when the mesh is redefined with end().
     * @return Pointer to the vertex array, nullptr before the mesh is set up with end().
     */
    VertexArray *vertexArray();

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
    std::shared_ptr<Normals> normals() const;

    /**
     * @brief Get the vertices array.
     * @return std::shared_ptr<Vertices> Vertices array.
     */
    std::shared_ptr<Vertices> vertices();

    /**
     * @brief The vertex colors (one RGBA row per vertex); call updateColors() after changing them.
     */
    [[nodiscard]] std::shared_ptr<Colors> colors() const;

    /** @brief Number of vertices of the mesh. */
    [[nodiscard]] GLuint vertexCount() const;

    /**
     * @brief Get the indices array.
     * @return std::shared_ptr<Indices> Indices array.
     */
    std::shared_ptr<Indices> indices() const;

    /** @brief OpenGL primitive type (GL_TRIANGLES, GL_LINES, ...). */
    [[nodiscard]] GLuint primType() const
    {
        return m_primType;
    }

    /**
     * @brief Bounding volume hierarchy of the mesh's triangles, for ray casts (see pick()).
     *
     * Built on first use and again after the geometry changed (end(), updateVertices()).
     * Null for meshes without triangles (lines, points). Needs no OpenGL context.
     */
    [[nodiscard]] MeshBvhPtr bvh();

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
