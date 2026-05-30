#include <ivf/billboard.h>
#include <ivf/transform_manager.h>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace ivf {

static const char* k_billVert = R"(
#version 330 core
layout(location = 0) in vec2 aQuadPos;  // -0.5..0.5
layout(location = 1) in vec2 aUV;

out vec2 vUV;

uniform mat4 viewProj;
uniform mat4 view;
uniform vec3 worldPos;
uniform vec2 size;       // (width, height)
uniform bool axisLocked; // cylindrical: only rotate around Y

void main() {
    vec3 right, up;
    if (axisLocked) {
        right = normalize(vec3(view[0][0], 0.0, view[2][0]));
        up    = vec3(0.0, 1.0, 0.0);
    } else {
        right = vec3(view[0][0], view[1][0], view[2][0]);
        up    = vec3(view[0][1], view[1][1], view[2][1]);
    }
    vec3 pos = worldPos
             + right * aQuadPos.x * size.x
             + up    * aQuadPos.y * size.y;
    vUV         = aUV;
    gl_Position = viewProj * vec4(pos, 1.0);
}
)";

static const char* k_billFrag = R"(
#version 330 core
in vec2 vUV;
out vec4 fragColor;
uniform sampler2D tex;
uniform bool useTexture;
uniform vec4 color;
void main() {
    vec4 c = color;
    if (useTexture) c *= texture(tex, vUV);
    if (c.a < 0.01) discard;
    fragColor = c;
}
)";

struct BillVertex { float x, y, u, v; };
static const BillVertex k_quad[6] = {
    {-0.5f,-0.5f, 0,0}, { 0.5f,-0.5f, 1,0}, { 0.5f, 0.5f, 1,1},
    {-0.5f,-0.5f, 0,0}, { 0.5f, 0.5f, 1,1}, {-0.5f, 0.5f, 0,1},
};

Billboard::Billboard(float width, float height)
    : m_width(width), m_height(height)
{}

Billboard::~Billboard()
{
    if (m_vao)     glDeleteVertexArrays(1, &m_vao);
    if (m_vbo)     glDeleteBuffers(1, &m_vbo);
    if (m_program) glDeleteProgram(m_program);
}

std::shared_ptr<Billboard> Billboard::create(float width, float height)
{
    return std::make_shared<Billboard>(width, height);
}

void Billboard::setUVRegion(int col, int row, int cols, int rows)
{
    m_uvMin = { float(col)   / float(cols), float(row)   / float(rows) };
    m_uvMax = { float(col+1) / float(cols), float(row+1) / float(rows) };
}

void Billboard::initGPU()
{
    if (m_gpuReady) return;

    auto compile = [](GLenum type, const char* src) -> GLuint {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        return s;
    };

    GLuint vert = compile(GL_VERTEX_SHADER,   k_billVert);
    GLuint frag = compile(GL_FRAGMENT_SHADER, k_billFrag);
    m_program = glCreateProgram();
    glAttachShader(m_program, vert);
    glAttachShader(m_program, frag);
    glLinkProgram(m_program);
    glDeleteShader(vert);
    glDeleteShader(frag);

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(k_quad), k_quad, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(BillVertex),
                          reinterpret_cast<void*>(offsetof(BillVertex, x)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BillVertex),
                          reinterpret_cast<void*>(offsetof(BillVertex, u)));
    glBindVertexArray(0);

    m_gpuReady = true;
}

void Billboard::doSetup()
{
    initGPU();
}

void Billboard::doDraw()
{
    if (!m_gpuReady) initGPU();

    auto& xfm = *TransformManager::instance();
    glm::mat4 vp = xfm.projectionMatrix() * xfm.viewMatrix();

    glUseProgram(m_program);
    glUniformMatrix4fv(glGetUniformLocation(m_program, "viewProj"), 1, GL_FALSE, glm::value_ptr(vp));
    glUniformMatrix4fv(glGetUniformLocation(m_program, "view"),     1, GL_FALSE, glm::value_ptr(xfm.viewMatrix()));
    glUniform3fv(glGetUniformLocation(m_program, "worldPos"), 1, glm::value_ptr(m_pos));
    glUniform2f(glGetUniformLocation(m_program, "size"), m_width, m_height);
    glUniform1i(glGetUniformLocation(m_program, "axisLocked"), m_axisLocked ? 1 : 0);
    glUniform4fv(glGetUniformLocation(m_program, "color"), 1, glm::value_ptr(m_color));
    glUniform1i(glGetUniformLocation(m_program, "useTexture"), (m_texture ? 1 : 0));

    if (m_texture) {
        glActiveTexture(GL_TEXTURE0);
        m_texture->bind();
        glUniform1i(glGetUniformLocation(m_program, "tex"), 0);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    if (m_texture) m_texture->unbind();
    glUseProgram(0);
}

} // namespace ivf
