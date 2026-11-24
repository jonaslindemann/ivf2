#include <ivf/instancing.h>

#include <ivf/vertex_array.h>

#include <cstddef>

using namespace ivf;

void ivf::bindInstanceAttributes(VertexArray &vao, GLuint buffer)
{
    vao.bind();
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    const GLsizei stride = sizeof(InstanceAttributes);
    for (GLuint col = 0; col < 4; ++col)
    {
        glEnableVertexAttribArray(InstanceTransformLocation + col);
        glVertexAttribPointer(
            InstanceTransformLocation + col, 4, GL_FLOAT, GL_FALSE, stride,
            reinterpret_cast<void *>(offsetof(InstanceAttributes, transform) + col * sizeof(glm::vec4)));
        glVertexAttribDivisor(InstanceTransformLocation + col, 1);
    }

    glEnableVertexAttribArray(InstanceColorLocation);
    glVertexAttribPointer(InstanceColorLocation, 4, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void *>(offsetof(InstanceAttributes, color)));
    glVertexAttribDivisor(InstanceColorLocation, 1);

    glEnableVertexAttribArray(InstanceDataLocation);
    glVertexAttribPointer(InstanceDataLocation, 4, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void *>(offsetof(InstanceAttributes, data)));
    glVertexAttribDivisor(InstanceDataLocation, 1);

    vao.unbind();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
