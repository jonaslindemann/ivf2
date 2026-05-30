#include <ivf/skybox.h>
#include <ivf/shader_manager.h>
#include <ivf/transform_manager.h>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ivf {

static const char* k_skyboxVert = R"(
#version 330 core
out vec3 vTexCoord;
uniform mat4 inverseProjection;
uniform mat4 inverseView;
void main() {
    const vec2 positions[3] = vec2[3](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );

    vec2 ndc = positions[gl_VertexID];
    vec4 viewPos = inverseProjection * vec4(ndc, 1.0, 1.0);
    vec3 viewDir = viewPos.xyz / viewPos.w;
    vTexCoord = mat3(inverseView) * viewDir;

    gl_Position = vec4(ndc, 1.0, 1.0); // force depth to 1.0 (far plane)
}
)";

static const char* k_skyboxFrag = R"(
#version 330 core
in vec3 vTexCoord;
out vec4 fragColor;
uniform samplerCube skybox;
uniform float exposure;
void main() {
    vec3 color = texture(skybox, normalize(vTexCoord)).rgb * exposure;
    fragColor  = vec4(color, 1.0);
}
)";

Skybox::Skybox(CubemapPtr cubemap)
    : m_cubemap(std::move(cubemap))
{}

Skybox::~Skybox()
{
    if (m_vao)     glDeleteVertexArrays(1, &m_vao);
    if (m_program) glDeleteProgram(m_program);
}

std::shared_ptr<Skybox> Skybox::create(CubemapPtr cubemap)
{
    return std::make_shared<Skybox>(std::move(cubemap));
}

void Skybox::initGPU()
{
    if (m_gpuReady) return;

    auto compile = [](GLenum type, const char* src) -> GLuint {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        return s;
    };

    GLuint vert = compile(GL_VERTEX_SHADER,   k_skyboxVert);
    GLuint frag = compile(GL_FRAGMENT_SHADER, k_skyboxFrag);
    m_program = glCreateProgram();
    glAttachShader(m_program, vert);
    glAttachShader(m_program, frag);
    glLinkProgram(m_program);
    glDeleteShader(vert);
    glDeleteShader(frag);

    glGenVertexArrays(1, &m_vao);

    m_gpuReady = true;
}

void Skybox::doSetup()
{
    initGPU();
}

void Skybox::doDraw()
{
    if (!m_cubemap) return;
    if (!m_gpuReady) initGPU();

    auto& xfm = *TransformManager::instance();

    // Remove translation from view matrix (keep only rotation)
    glm::mat4 view = glm::mat4(glm::mat3(xfm.viewMatrix()));
    glm::mat4 inverseProjection = glm::inverse(xfm.projectionMatrix());
    glm::mat4 inverseView = glm::inverse(view);

    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);

    glUseProgram(m_program);
    glUniformMatrix4fv(glGetUniformLocation(m_program, "inverseProjection"), 1, GL_FALSE,
                       glm::value_ptr(inverseProjection));
    glUniformMatrix4fv(glGetUniformLocation(m_program, "inverseView"), 1, GL_FALSE,
                       glm::value_ptr(inverseView));
    glUniform1f(glGetUniformLocation(m_program, "exposure"), m_exposure);

    m_cubemap->bind(0);
    glUniform1i(glGetUniformLocation(m_program, "skybox"), 0);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    m_cubemap->unbind();

    auto currentProgram = ShaderManager::instance()->currentProgram();
    if (currentProgram)
        currentProgram->use();

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

} // namespace ivf
