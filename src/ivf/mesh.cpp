#include <ivf/mesh.h>
#include <ivf/render_context.h>

#include <memory>
#include <iostream>

#include <glad/glad.h>

#include <ivf/shader_manager.h>
#include <ivf/transform_manager.h>
#include <ivf/utils.h>
#include <ivf/material.h>
#include <ivf/instancing.h>
#include <ivf/program.h>

using namespace ivf;
using namespace std;

glm::vec3 computeNormal(glm::vec3 const &a, glm::vec3 const &b, glm::vec3 const &c)
{
    return glm::normalize(glm::cross(c - a, b - a));
}

Mesh::Mesh(GLuint vsize, GLuint isize, GLuint primType, GLenum usage)
    : m_primType(primType), m_wireframe(false), m_position(0.0f), m_generateNormals(true), m_enabled(true),
      m_polygonOffsetFactor(0.0f), m_polygonOffsetUnits(0.0f), m_depthFunc(GL_LESS), m_lineWidth(1.0f), m_usage(usage)
{
    this->setSize(vsize, isize);
}

Mesh::~Mesh()
{
    for (auto &copy : m_gpu)
        release(*copy);
}

std::shared_ptr<Mesh> ivf::Mesh::create(GLuint vsize, GLuint isize)
{
    return std::make_shared<Mesh>(vsize, isize);
}

void Mesh::setupPrim()
{
    if (m_primType == GL_TRIANGLES)
        m_indices = std::make_shared<Indices>(m_indexSize, 3);
    // if (m_primType == GL_QUADS)
    //	m_indices = std::make_shared<Indices>(new Indices(m_indexSize, 4));
    if (m_primType == GL_LINES)
        m_indices = std::make_shared<Indices>(m_indexSize, 2);
    if (m_primType == GL_LINE_STRIP)
        m_indices = std::make_shared<Indices>(m_indexSize, 1);
    if (m_primType == GL_LINE_LOOP)
        m_indices = std::make_shared<Indices>(m_indexSize, 1);
    if (m_primType == GL_TRIANGLE_STRIP)
        m_indices = std::make_shared<Indices>(m_indexSize, 1);
    if (m_primType == GL_TRIANGLE_FAN)
        m_indices = std::make_shared<Indices>(m_indexSize, 1);
    // if (m_primType == GL_QUAD_STRIP)
    //	m_indices = std::make_unique<Indices>(new Indices(m_indexSize, 1));
    // if (m_primType == GL_POLYGON)
    //	m_indices = std::make_unique<Indices>(new Indices(m_indexSize, 1));
    if (m_primType == GL_POINTS)
        m_indices = std::make_shared<Indices>(m_indexSize, 1);
}

void Mesh::setSize(GLuint vsize, GLuint isize)
{
    m_verts = std::make_shared<Vertices>(vsize);
    m_colors = std::make_shared<Colors>(vsize);
    m_normals = std::make_shared<Normals>(vsize);
    m_texCoords = std::make_shared<TexCoords>(vsize);

    m_glVerts = nullptr;
    m_glColors = nullptr;
    m_glNormals = nullptr;

    m_indexSize = isize;

    if (m_indexSize != 0)
        this->setupPrim();
    else
        m_indices = nullptr;

    m_vertPos = 0;
    m_colorPos = 0;
    m_indexPos = 0;
    m_normalPos = 0;
    m_texCoordPos = 0;

    // Locations every ShaderManager program binds before linking (no GL context needed)
    m_vertexAttrId = 0;   // aPos
    m_colorAttrId = 1;    // aColor
    m_texCoordAttrId = 2; // aTex
    m_normalAttrId = 3;   // aNormal
}

void ivf::Mesh::setEnabled(bool flag)
{
    m_enabled = flag;
}

bool ivf::Mesh::enabled() const
{
    return m_enabled;
}

void ivf::Mesh::setGenerateNormals(bool flag)
{
    m_generateNormals = flag;
}

bool ivf::Mesh::generateNormals() const
{
    return m_generateNormals;
}

void ivf::Mesh::setPolygonOffset(float factor, float units)
{
    m_polygonOffsetFactor = factor;
    m_polygonOffsetUnits = units;
}

float ivf::Mesh::polygonOffsetFactor() const
{
    return m_polygonOffsetFactor;
}

float ivf::Mesh::polygonOffsetUnits() const
{
    return m_polygonOffsetUnits;
}

void ivf::Mesh::setDepthFunc(GLenum func)
{
    m_depthFunc = func;
}

GLenum ivf::Mesh::depthFunc() const
{
    return m_depthFunc;
}

void ivf::Mesh::setLineWidth(GLfloat width)
{
    m_lineWidth = width;
}

GLfloat ivf::Mesh::lineWidth() const
{
    return m_lineWidth;
}

void ivf::Mesh::setWireframe(bool flag)
{
    m_wireframe = flag;
}

bool ivf::Mesh::wireframe() const
{
    return m_wireframe;
}

void Mesh::setVertexAttrId(GLuint id)
{
    m_vertexAttrId = id;
}

void Mesh::setColorAttrId(GLuint id)
{
    m_colorAttrId = id;
}

void Mesh::setNormalAttrId(GLuint id)
{
    m_normalAttrId = id;
}

void ivf::Mesh::setTexCoordAttrId(GLuint id)
{
    m_texCoordAttrId = id;
}

void Mesh::begin(GLuint primType)
{
    m_primType = primType;

    if (m_indexSize != 0)
        this->setupPrim();
}

void Mesh::vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    m_verts->setVertex(m_vertPos++, x, y, z);
}

void Mesh::vertex3d(GLdouble x, GLdouble y, GLdouble z)
{
    m_verts->setVertex(m_vertPos++, GLfloat(x), GLfloat(y), GLfloat(z));
}

void Mesh::vertex3d(const glm::dvec3 &v)
{
    this->vertex3d(v.x, v.y, v.z);
}

void ivf::Mesh::vertex3f(const glm::vec3 v)
{
    this->vertex3f(v.x, v.y, v.z);
}

void ivf::Mesh::tex2f(GLfloat s, GLfloat t)
{
    m_texCoords->setTexCoord(m_texCoordPos++, s, t);
}

void ivf::Mesh::normal3f(GLfloat vx, GLfloat vy, GLfloat vz)
{
    m_normals->setNormal(m_normalPos++, vx, vy, vz);
}

void ivf::Mesh::normal3f(const glm::vec3 v)
{
    this->normal3f(v.x, v.y, v.z);
}

void ivf::Mesh::normal3d(GLdouble vx, GLdouble vy, GLdouble vz)
{
    this->normal3f(GLfloat(vx), GLfloat(vy), GLfloat(vz));
}

void Mesh::vertex2f(GLfloat x, GLfloat y)
{
    m_verts->setVertex(m_vertPos++, x, y, 0.0);
}

void Mesh::vertex2d(GLdouble x, GLdouble y)
{
    m_verts->setVertex(m_vertPos++, GLfloat(x), GLfloat(y), 0.0f);
}

void Mesh::color3f(GLfloat r, GLfloat g, GLfloat b)
{
    m_colors->setColor(m_colorPos++, r, g, b);
}

void Mesh::color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    m_colors->setColor(m_colorPos++, r, g, b, a);
}

void Mesh::index1i(GLuint i0)
{
    if (m_indices != 0)
        m_indices->setIndex(m_indexPos++, i0);
}

void Mesh::index2i(GLuint i0, GLuint i1)
{
    if (m_indices != 0)
        m_indices->setIndex(m_indexPos++, i0, i1);
}

void Mesh::index3i(GLuint i0, GLuint i1, GLuint i2)
{
    if (m_indices != 0)
        m_indices->setIndex(m_indexPos++, i0, i1, i2);
}

void Mesh::index4i(GLuint i0, GLuint i1, GLuint i2, GLuint i3)
{
    if (m_indices != 0)
        m_indices->setIndex(m_indexPos++, i0, i1, i2, i3);
}

void Mesh::triQuad(GLdouble w, GLdouble h, GLdouble offset, GLdouble vx, GLdouble vy, GLdouble vz)
{
    glm::dvec3 ez(glm::dvec3(vx, vy, vz));
    glm::dvec3 ezp(glm::dvec3(vx, 0.0, vz));
    ezp = glm::normalize(ezp);
    glm::dvec3 ex(-ezp.z, 0.0, ezp.x);
    glm::dvec3 ey = glm::cross(ez, ex);

    glm::dvec3 p0 = (-w / 2.0) * ex + (-h / 2.0) * ey + (offset)*ez;
    glm::dvec3 p1 = (w / 2.0) * ex + (-h / 2.0) * ey + (offset)*ez;
    glm::dvec3 p2 = (w / 2.0) * ex + (h / 2.0) * ey + (offset)*ez;

    this->vertex3d(p0);
    this->vertex3d(p1);
    this->vertex3d(p2);

    p0 = (-w / 2.0) * ex + (-h / 2.0) * ey + (offset)*ez;
    p1 = (-w / 2.0) * ex + (h / 2.0) * ey + (offset)*ez;
    p2 = (w / 2.0) * ex + (h / 2.0) * ey + (offset)*ez;

    this->vertex3d(p0);
    this->vertex3d(p1);
    this->vertex3d(p2);
}

void Mesh::end()
{
    // Compute actual gl arrays

    if (m_primType == GL_TRIANGLES)
    {
        glm::vec3 points[3];
        glm::vec3 norm;

        if (m_indices != nullptr)
        {
            if (m_generateNormals)
            {
                for (GLuint i = 0; i < m_indices->rows(); i++)
                {
                    points[0] = m_verts->vertex(m_indices->at(i, 0));
                    points[1] = m_verts->vertex(m_indices->at(i, 1));
                    points[2] = m_verts->vertex(m_indices->at(i, 2));
                    norm = computeNormal(points[0], points[1], points[2]);
                    m_normals->setNormal(m_indices->at(i, 0), -norm);
                    m_normals->setNormal(m_indices->at(i, 1), -norm);
                    m_normals->setNormal(m_indices->at(i, 2), -norm);
                }
            }
        }
    }

    // Which triangle winding faces outwards: generator meshes are mirrored (Y/Z swap) and
    // clockwise, hand-built ones usually counter-clockwise. Decided by the vertex normals.
    m_clockwise = false;
    if (m_primType == GL_TRIANGLES && m_verts && m_normals)
    {
        long votes = 0; // > 0: counter-clockwise front faces
        auto vote = [&](GLuint i0, GLuint i1, GLuint i2) {
            const glm::vec3 a = m_verts->vertex(i0), b = m_verts->vertex(i1), c = m_verts->vertex(i2);
            const glm::vec3 n = m_normals->normal(i0) + m_normals->normal(i1) + m_normals->normal(i2);
            const float d = glm::dot(glm::cross(b - a, c - a), n);
            votes += d > 0.0f ? 1 : (d < 0.0f ? -1 : 0);
        };
        if (m_indices != nullptr)
        {
            for (GLuint i = 0; i < m_indices->rows(); i++)
                vote(m_indices->at(i, 0), m_indices->at(i, 1), m_indices->at(i, 2));
        }
        else
        {
            for (GLuint i = 0; i + 2 < m_verts->rows(); i += 3)
                vote(i, i + 1, i + 2);
        }
        m_clockwise = votes < 0;
    }

    ++m_dataVersion; // uploaded before the next draw (see gpuCopy())
}

void ivf::Mesh::updateVertices()
{
    ++m_vertexVersion;
}

void ivf::Mesh::updateColors()
{
    ++m_colorVersion;
}

void ivf::Mesh::updateNormals()
{
    glm::vec3 points[3];
    glm::vec3 norm;

    if (m_indices != nullptr)
    {
        for (GLuint i = 0; i < m_indices->rows(); i++)
        {
            points[0] = m_verts->vertex(m_indices->at(i, 0));
            points[1] = m_verts->vertex(m_indices->at(i, 1));
            points[2] = m_verts->vertex(m_indices->at(i, 2));
            norm = computeNormal(points[0], points[1], points[2]);
            m_normals->setNormal(m_indices->at(i, 0), norm);
            m_normals->setNormal(m_indices->at(i, 1), norm);
            m_normals->setNormal(m_indices->at(i, 2), norm);
        }
        ++m_normalVersion;
    }
}

Mesh::GpuCopy *Mesh::gpuCopy(RenderContext &renderContext)
{
    if (m_dataVersion == 0)
        return nullptr; // nothing defined yet

    RenderContext *context = &renderContext;

    // Copies of destroyed contexts were freed with them: just forget them
    std::erase_if(m_gpu, [](const std::unique_ptr<GpuCopy> &copy) {
        if (!copy->lifetime.expired())
            return false;
        release(*copy);
        return true;
    });

    GpuCopy *copy = nullptr;
    for (auto &candidate : m_gpu)
        if (candidate->context == context)
            copy = candidate.get();

    if (copy == nullptr)
    {
        m_gpu.push_back(std::make_unique<GpuCopy>());
        copy = m_gpu.back().get();
        copy->context = context;
        copy->lifetime = context->lifetime();
    }

    if (copy->dataVersion != m_dataVersion)
    {
        upload(*copy);
    }
    else
    {
        if (copy->vertexVersion != m_vertexVersion && copy->vertices)
        {
            copy->vertices->updateArray(m_verts.get());
            copy->vertexVersion = m_vertexVersion;
        }
        if (copy->normalVersion != m_normalVersion && copy->normals)
        {
            copy->normals->updateArray(m_normals.get());
            copy->normalVersion = m_normalVersion;
        }
        if (copy->colorVersion != m_colorVersion && copy->colors)
        {
            copy->colors->updateArray(m_colors.get());
            copy->colorVersion = m_colorVersion;
        }
    }
    return copy;
}

void Mesh::upload(GpuCopy &copy)
{
    // The previous objects of this context are replaced; the new vertex array is created
    // before the old one is deleted, so its address changes (InstancedMeshNode relies on it).
    GpuCopy previous;
    previous.vao = std::move(copy.vao);
    previous.vertices = std::move(copy.vertices);
    previous.colors = std::move(copy.colors);
    previous.normals = std::move(copy.normals);
    previous.texCoords = std::move(copy.texCoords);
    previous.indices = std::move(copy.indices);

    ivf::clearError(); // flush any accumulated GL errors from before the upload
    copy.vao = std::make_unique<VertexArray>();
    copy.vao->bind();

    copy.vertices = std::make_unique<VertexBuffer>(m_usage);
    copy.vertices->setArray(m_verts.get());

    glEnableVertexAttribArray(static_cast<GLuint>(m_vertexAttrId));
    glVertexAttribPointer(static_cast<GLuint>(m_vertexAttrId), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    if (m_indices != 0)
    {
        copy.indices = std::make_unique<IndexBuffer>();
        copy.indices->setArray(m_indices.get());
    }

    if (m_colorAttrId != -1)
    {
        copy.colors = std::make_unique<VertexBuffer>(m_usage);
        copy.colors->setArray(m_colors.get());
        glEnableVertexAttribArray(static_cast<GLuint>(m_colorAttrId));
        glVertexAttribPointer(static_cast<GLuint>(m_colorAttrId), 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
    }

    if (m_normalAttrId != -1)
    {
        copy.normals = std::make_unique<VertexBuffer>(m_usage);
        copy.normals->setArray(m_normals.get());
        glEnableVertexAttribArray(static_cast<GLuint>(m_normalAttrId));
        glVertexAttribPointer(static_cast<GLuint>(m_normalAttrId), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    }

    if (m_texCoordAttrId != -1)
    {
        copy.texCoords = std::make_unique<VertexBuffer>(m_usage);
        copy.texCoords->setArray(m_texCoords.get());
        glEnableVertexAttribArray(static_cast<GLuint>(m_texCoordAttrId));
        glVertexAttribPointer(static_cast<GLuint>(m_texCoordAttrId), 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    }

    copy.vao->unbind();
    checkPrintError("Mesh", __FILE__, __LINE__);

    copy.dataVersion = m_dataVersion;
    copy.vertexVersion = m_vertexVersion;
    copy.normalVersion = m_normalVersion;
    copy.colorVersion = m_colorVersion;
}

void Mesh::release(GpuCopy &copy)
{
    std::vector<GLuint> buffers;
    GLuint vao = copy.vao ? copy.vao->release() : 0;
    for (auto *buffer : {copy.vertices.get(), copy.colors.get(), copy.normals.get(), copy.texCoords.get()})
        if (buffer != nullptr)
            buffers.push_back(buffer->release());
    if (copy.indices)
        buffers.push_back(copy.indices->release());
    copy.vao.reset();
    copy.vertices.reset();
    copy.colors.reset();
    copy.normals.reset();
    copy.texCoords.reset();
    copy.indices.reset();

    // A destroyed context took its objects with it
    if (copy.lifetime.expired() || copy.context == nullptr)
        return;

    auto destroy = [vao, buffers]() {
        if (vao != 0)
            glDeleteVertexArrays(1, &vao);
        if (!buffers.empty())
            glDeleteBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
    };
    if (copy.context->isCurrent())
        destroy();
    else
        copy.context->deleteLater(destroy); // runs when that context is current again
}

void Mesh::draw()
{
    drawImpl(*RenderContext::currentOrFallback(), 0);
}

void Mesh::draw(RenderContext &context)
{
    drawImpl(context, 0);
}

void Mesh::drawInstanced(GLsizei instanceCount)
{
    if (instanceCount > 0)
        drawImpl(*RenderContext::currentOrFallback(), instanceCount);
}

void Mesh::drawInstanced(RenderContext &context, GLsizei instanceCount)
{
    if (instanceCount > 0)
        drawImpl(context, instanceCount);
}

VertexArray *Mesh::vertexArray()
{
    GpuCopy *copy = gpuCopy(*RenderContext::currentOrFallback());
    return copy != nullptr ? copy->vao.get() : nullptr;
}

void Mesh::drawInstances(RenderContext &context, std::span<const glm::mat4> transforms)
{
    if (!transforms.empty())
        drawImpl(context, 0, transforms);
}

void Mesh::drawImpl(RenderContext &context, GLsizei instanceCount, std::span<const glm::mat4> transforms)
{
    GpuCopy *gpu = gpuCopy(context);
    if (gpu == nullptr)
        return; // never defined with end()

    if (m_material != nullptr)
        m_material->apply();

    if ((m_polygonOffsetFactor != 0.0f) || (m_polygonOffsetUnits != 0.0f))
    {
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(m_polygonOffsetFactor, m_polygonOffsetUnits);
    }
    else
    {
        glDisable(GL_POLYGON_OFFSET_LINE);
    }

    if (m_depthFunc != GL_LESS)
        glDepthFunc(m_depthFunc);

    // Widths above 1 are an error in forward-compatible contexts (see wideLinesSupported()).
    const bool wideLine = m_lineWidth != 1.0f && context.wideLinesSupported();
    if (wideLine)
        glLineWidth(m_lineWidth);

    // Front faces follow the mesh's winding (matters when face culling is enabled)
    if (m_clockwise)
        glFrontFace(GL_CW);

    // OpenGL ES (WebGL) has no polygon modes: wireframe meshes are drawn filled there
    if (!RenderContext::isGles())
        glPolygonMode(GL_FRONT_AND_BACK, m_wireframe ? GL_LINE : GL_FILL);

    auto drawCall = [&](GLsizei instances) {
        gpu->vao->bind();
        // GL_ERR expands to several statements, so keep the braces
        if (m_indices != nullptr)
        {
            if (instances > 0)
            {
                GL_ERR(glDrawElementsInstanced(m_primType, m_indices->size(), GL_UNSIGNED_INT, 0, instances));
            }
            else
            {
                GL_ERR(glDrawElements(m_primType, m_indices->size(), GL_UNSIGNED_INT, 0));
            }
        }
        else
        {
            if (instances > 0)
            {
                GL_ERR(glDrawArraysInstanced(m_primType, 0, m_verts->rows(), instances));
            }
            else
            {
                GL_ERR(glDrawArrays(m_primType, 0, m_verts->rows()));
            }
        }
        gpu->vao->unbind();
        context.countDrawCall();
    };

    if (transforms.empty())
    {
        drawCall(instanceCount);
    }
    else
    {
        // Copies of the mesh at several places: one instanced draw if the program (after the
        // mesh's own material) reads the instance attributes, else one draw per copy
        auto *xfm = context.transforms();
        const glm::mat4 model = xfm->modelMatrix();
        ProgramPtr program = context.shaders()->currentProgram();
        const GLint useInstancing = program ? program->uniformLoc("useInstancing") : -1;
        if (useInstancing != -1)
        {
            bindInstanceAttributes(*gpu->vao, context.uploadInstanceTransforms(transforms));
            xfm->setModelMatrix(glm::mat4(1.0f)); // the shader computes model * aInstanceModel
            program->uniformBool(useInstancing, true);
            drawCall(static_cast<GLsizei>(transforms.size()));
            program->uniformBool(useInstancing, false);
            context.countInstances(transforms.size());
        }
        else
        {
            for (const glm::mat4 &transform : transforms)
            {
                xfm->setModelMatrix(transform);
                drawCall(0);
            }
        }
        xfm->setModelMatrix(model);
    }

    if (m_clockwise)
        glFrontFace(GL_CCW);

    if (m_depthFunc != GL_LESS)
        glDepthFunc(GL_LESS);

    if (m_lineWidth != 1.0f)
        glLineWidth(1.0f);

    // Undo the mesh's own material (e.g. restore the program a ShaderMaterial replaced)
    if (m_material != nullptr)
        m_material->unapply();
}

void ivf::Mesh::drawAsPrim(GLuint prim)
{
    RenderContext &context = *RenderContext::currentOrFallback();
    GpuCopy *gpu = gpuCopy(context);
    if (gpu == nullptr)
        return;
    gpu->vao->bind();
    glDrawArrays(prim, 0, static_cast<GLsizei>(m_verts->rows()));
    gpu->vao->unbind();
    context.countDrawCall();
}

void ivf::Mesh::setPos(glm::vec3 pos)
{
    m_position = pos;
}

glm::vec3 ivf::Mesh::pos()
{
    return m_position;
}

int ivf::Mesh::vertPos() const
{
    return m_vertPos;
}

int ivf::Mesh::colorPos() const
{
    return m_colorPos;
}

int ivf::Mesh::normalPos() const
{
    return m_normalPos;
}

int ivf::Mesh::indexPos() const
{
    return m_indexPos;
}

int ivf::Mesh::texCoordPos() const
{
    return m_texCoordPos;
}

GLuint ivf::Mesh::currentIndexPos()
{
    return m_indexPos;
}

std::shared_ptr<Normals> ivf::Mesh::normals() const
{
    return m_normals;
}

std::shared_ptr<Vertices> ivf::Mesh::vertices()
{
    return m_verts;
}

std::shared_ptr<Colors> ivf::Mesh::colors() const
{
    return m_colors;
}

GLuint ivf::Mesh::vertexCount() const
{
    return m_verts ? m_verts->rows() : 0;
}

std::shared_ptr<Indices> ivf::Mesh::indices() const
{
    return m_indices;
}

MeshBvhPtr ivf::Mesh::bvh()
{
    if (m_bvh && m_bvhDataVersion == m_dataVersion && m_bvhVertexVersion == m_vertexVersion)
        return m_bvh;
    m_bvhDataVersion = m_dataVersion;
    m_bvhVertexVersion = m_vertexVersion;
    m_bvh.reset();
    if (!m_verts || m_dataVersion == 0)
        return nullptr;

    const GLuint vertexCount = m_verts->rows();
    std::vector<glm::vec3> positions(vertexCount);
    for (GLuint i = 0; i < vertexCount; ++i)
        positions[i] = m_verts->vertex(i);

    // The vertex order the mesh draws (indexed or not)
    std::vector<GLuint> order;
    if (m_indices)
    {
        const GLuint cols = m_indices->cols();
        order.reserve(static_cast<std::size_t>(m_indices->rows()) * cols);
        for (GLuint r = 0; r < m_indices->rows(); ++r)
            for (GLuint c = 0; c < cols; ++c)
                order.push_back(m_indices->at(r, c));
    }
    else
    {
        order.resize(vertexCount);
        for (GLuint i = 0; i < vertexCount; ++i)
            order[i] = i;
    }

    std::vector<glm::uvec3> triangles;
    switch (m_primType)
    {
    case GL_TRIANGLES:
        for (std::size_t i = 0; i + 2 < order.size(); i += 3)
            triangles.emplace_back(order[i], order[i + 1], order[i + 2]);
        break;
    case GL_TRIANGLE_STRIP:
        for (std::size_t i = 2; i < order.size(); ++i)
            triangles.emplace_back(order[i - 2], order[i - 1], order[i]);
        break;
    case GL_TRIANGLE_FAN:
        for (std::size_t i = 2; i < order.size(); ++i)
            triangles.emplace_back(order[0], order[i - 1], order[i]);
        break;
    case GL_QUADS:
        for (std::size_t i = 0; i + 3 < order.size(); i += 4)
        {
            triangles.emplace_back(order[i], order[i + 1], order[i + 2]);
            triangles.emplace_back(order[i], order[i + 2], order[i + 3]);
        }
        break;
    default:
        return nullptr; // lines and points cannot be hit
    }
    if (triangles.empty())
        return nullptr;
    m_bvh = std::make_shared<const MeshBvh>(std::move(positions), std::move(triangles));
    return m_bvh;
}

ivf::MaterialPtr ivf::Mesh::material() const
{
    return m_material;
}

void ivf::Mesh::setMaterial(ivf::MaterialPtr material)
{
    m_material = material;
}

void ivf::Mesh::print()
{
    if (m_indices != nullptr)
    {
        // GL_ERR(glDrawElements(m_primType, m_indices->size(), GL_UNSIGNED_INT, 0));
    }
    else
    {
        // GL_ERR(glDrawArrays(m_primType, 0, m_verts->rows()));
        cout << "---------" << endl;
        cout << "Vertices:" << endl;
        if (m_verts != nullptr)
            m_verts->print();
        cout << "Normals:" << endl;
        if (m_normals != nullptr)
            m_normals->print();
        cout << "Colors:" << endl;
        if (m_colors != nullptr)
            m_colors->print();
    }
}
